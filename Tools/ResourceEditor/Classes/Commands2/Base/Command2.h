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


#ifndef __COMMAND2_H__
#define __COMMAND2_H__

#include "Base/BaseTypes.h"
#include "Scene3D/Entity.h"

#include "Command/ICommand.h"

#include "Commands2/CommandID.h"
#include "Commands2/Base/CommandNotify.h"

class Command2 : public CommandNotifyProvider, public DAVA::ICommand
{
public:
    using Pointer = std::unique_ptr<Command2>;

protected:
    Command2(DAVA::int32 id, const DAVA::String& text = "");

public:
    ~Command2() override;

    template <typename CMD, typename... Arg>
    static std::unique_ptr<CMD> Create(Arg&&... arg)
    {
        return std::unique_ptr<CMD>(new CMD(std::forward<Arg>(arg)...));
    }

    static Pointer CreateEmptyCommand()
    {
        return Pointer();
    }

    DAVA::int32 GetId() const;
    const DAVA::String& GetText() const;

    DAVA_DEPRECATED(virtual DAVA::Entity* GetEntity() const = 0);

    void Execute() override;

    virtual bool IsModifying() const;
    virtual bool CanUndo() const;

    virtual bool MatchCommandID(DAVA::int32 commandID) const;
    virtual bool MatchCommandIDs(const DAVA::Vector<DAVA::int32>& commandIDVector) const;

protected:
    void UndoInternalCommand(Command2* command);
    void RedoInternalCommand(Command2* command);

    const DAVA::String text;
    const DAVA::int32 id;
};

inline DAVA::int32 Command2::GetId() const
{
    return id;
}

inline const DAVA::String& Command2::GetText() const
{
    return text;
}

inline bool Command2::CanUndo() const
{
    return true;
}

inline bool Command2::IsModifying() const
{
    return true;
}

#endif // __COMMAND2_H__
