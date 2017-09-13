#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <stdio.h>
#include <string.h>

#include <event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/util.h>  
#include <thread>


const int PORT  = 9999;
void listener_cb(evconnlistener * listener,evutil_socket_t fd,struct sockaddr * sock,int socklen,void *arg);

void socket_read_cb(bufferevent * bev,void *arg);
void socket_event_cb(bufferevent *bev,short events ,void *arg);

int main()
{
	struct sockaddr_in sin;
	memset(&sin,0,sizeof(struct sockaddr_in));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(PORT);
	
	event_base *base = event_base_new();
	evconnlistener * listener =  evconnlistener_new_bind(base, listener_cb, base, LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE,10,(struct sockaddr*)&sin, sizeof(struct sockaddr_in));  
	
	event_base_dispatch(base);
	evconnlistener_free(listener);
	event_base_free(base);
	return 0;
}

void listener_cb(evconnlistener *listener,evutil_socket_t fd,struct sockaddr *sock,int socklen,void *arg)
{
	printf("accept a client %d\n",fd);
	
	event_base * base = (event_base *) arg;
	bufferevent *bev = bufferevent_socket_new(base,fd,BEV_OPT_CLOSE_ON_FREE);
	
	bufferevent_setcb(bev,socket_read_cb,NULL,socket_event_cb,NULL);
	bufferevent_enable(bev,EV_READ | EV_PERSIST);
	
}

void socket_read_cb(bufferevent *bev,void * arg)
{
	char msg[4096];
	size_t len = bufferevent_read(bev,msg,sizeof(msg) - 1);
	msg[len] = '\0';
	
	printf("server read the date %s\n",msg);
	char reply[] = "I have read your data";
	bufferevent_write(bev,reply,strlen(reply));
}

void socket_event_cb(bufferevent *bev, short events,void * arg)
{
	if(events & BEV_EVENT_EOF)
		printf("connettion closed\n");
	else if(events & BEV_EVENT_ERROR)
		printf("some other error\n");
	
	bufferevent_free(bev);
}
