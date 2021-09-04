#pragma once
using namespace System;
using namespace BrainAlive_Lib;


namespace Wrapper
{
public
    ref class BrainAliveLibWrapper
    {
        BrainAliveLibWrapper (void)
        {
            brainalive_obj = gcnew BrainAlive ();
        }

    public:
        BrainAlive ^brainalive_obj;
        static BrainAliveLibWrapper ^instance = gcnew BrainAliveLibWrapper ();
    };
}
