/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#ifndef __DAVAENGINE_TCPACCEPTOR_H__
#define __DAVAENGINE_TCPACCEPTOR_H__

#include <Functional/Function.h>

#include <Network/Base/TCPAcceptorTemplate.h>

namespace DAVA
{
namespace Net
{
/*
 Class TCPAcceptor provides a TCP acceptor type.
 TCPAcceptor allows to listen at specified port and accept incoming connection putting TCPSocket into connected state.
 All operations are executed asynchronously and all these operations must be started in thread context
 where IOLoop is running, i.e. they can be started during handler processing or using IOLoop Post method.
 List of methods starting async operations:
    StartListen
    Close

 TCPAcceptor notifies user about operation completion and its status through user-supplied functional objects (handlers or callbacks).
 Handlers are called with some parameters depending on operation. Each handler besides specific parameters is passed a
 pointer to TCPAcceptor instance.

 Note: handlers should not block, this will cause all network system to freeze.

 To start working with TCPAcceptor you should call one of the Bind methods and call StartListen and in its handler
 call Accept.

 Listen operation is started only once, it will be automatically restarted after executing handler. To stop listening
 call Close method.

 Close also is executed asynchronously and in its handler it is allowed to destroy TCPSocket object.
*/
class TCPSocket;
class TCPAcceptor : public TCPAcceptorTemplate<TCPAcceptor>
{
private:
    friend TCPAcceptorTemplate<TCPAcceptor>; // Make base class friend to allow it to call my Handle... methods

public:
    using CloseHandlerType = Function<void(TCPAcceptor* acceptor)>;
    using ConnectHandlerType = Function<void(TCPAcceptor* acceptor, int32 error)>;

public:
    TCPAcceptor(IOLoop* ioLoop);

    int32 StartListen(ConnectHandlerType handler, int32 backlog = SOMAXCONN);
    void Close(CloseHandlerType handler = CloseHandlerType());

    int32 Accept(TCPSocket* socket);

private:
    void HandleClose();
    void HandleConnect(int32 error);

private:
    CloseHandlerType closeHandler;
    ConnectHandlerType connectHandler;
};

} // namespace Net
} // namespace DAVA

#endif // __DAVAENGINE_TCPACCEPTOR_H__
