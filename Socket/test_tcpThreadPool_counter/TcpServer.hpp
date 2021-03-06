#pragma once

#include <iostream>
#include <string>
#include <cstring>
#include <strings.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <unordered_map>
#include "ThreadPool.h"

#include "comm.hpp"

using namespace std;
class Sock{

  private:
    int listensock;
    int port;
    string ip;
  public:
    Sock(const string& _ip,const int& _port)
      :ip(_ip),port(_port),listensock(-1)
    {
      ; 
    }
  void Socket()
  {
    listensock = socket(AF_INET,SOCK_STREAM,0);
    if(listensock<0)
    {
      cerr<<"socket"<<endl;
      exit(1);
    }
  }
  void Bind()
  {
    struct sockaddr_in addr;
    bzero(&addr,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port=htons(port);
   


    //服务器端ip可为0,一个服务器可能两个网卡，即2ip，如果有多个ip，可以设置一个宏，即服务器可接收发送到本机任意ip的数据，客户端不可以省略
    //addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_addr.s_addr= inet_addr(ip.c_str());
    
    if(bind(listensock,(struct sockaddr*)&addr,sizeof(addr))<0)
    {
        cerr<<"bind"<<endl;
        exit(3);
    }
    
  }

  void Listen()
  {
    if(listen(listensock,5)<0)
    {
      cerr<<"listen"<<endl;
      exit(4);
    }

  }

  int Accept()
  {
    struct sockaddr_in peer;
    socklen_t len = sizeof(peer);
    int sock =  accept(listensock,(struct sockaddr*)&peer,&len);
    if(sock<0)
    {
      cerr<<"accept "<<endl;
      return -1;
    }
    return sock;
    
  }

  ~Sock()
  {
      close(listensock);
  }
};

class Server
{
  private:
    Sock sock;
    ThreadPool pool;
  public:
    Server(const string& _ip,const int& _port):sock(_ip,_port),pool(5)
  {
  }
    void InitServer()
    {
      signal(SIGCHLD,SIG_IGN);//忽略
      sock.Socket();
      sock.Bind();
      sock.Listen();
      pool.InitThreadPool();
    }

    //tcp  recv  send
   static void Service(int sock)
    {
      requst r;
      Qrequst q;

      q.status = 0;
        recv(sock,&r,sizeof(r),0);
        switch(r.op)
        {
          case 0:
            q.result = r.x+r.y;
            break;

          case 1:
            q.result = r.x - r.y;
            break;

          case 2:
            q.result = r.x*r.y;
            break;
          case 3:
            if(r.y == 0)
            {
              q.status = 1;
                  
            }
            else
            {
              q.result = r.x/r.y;
            }

            break;
          case 4:
            break;
          default:
            q.status = 2;
            break;
        }

      send(sock,&q,sizeof(q),0);
     /* unordered_map<string,string> dict;
    dict.insert(make_pair("hello","你好"));
    dict.insert(make_pair("xust","西安科技大学"));
    dict.insert(make_pair("apple","苹果"));
    dict.insert(make_pair("hunter","猎人"));
      //udp 面向数据报  规定:sendto   recvfrom
      //tcp  面向字节流 read /write
      char buf[1024];
      for(;;)
      {
        ssize_t s = read(sock,buf,sizeof(buf)-1);
        if(s>0)
        {
          buf[s]=0;
          cout<<buf<<endl;
          string key = buf;
          string val = dict[key];
          if(val.empty())
          {
            val = "null";
          }
          write(sock,val.c_str(),val.size());

        }

        else if(s == 0)
        {
          cout<<"client is quite"<<endl;
          break;

        }
        else{
        cerr<<"read"<<endl;
        }

      }*/
      close(sock);
    }
    
    void Run()
    {
      for(;;)
      {
        int new_sock = sock.Accept();
        if(new_sock<0)
        {
          continue;
        }
        cout<<"Get a New Link "<<endl;;
        
        Task t(new_sock,Server::Service);
        pool.AddTask(t);
         
       //close(new_sock);
        
       /////////////////////////////

        
        
      }
    }

   ~Server()
   {
    }

};
