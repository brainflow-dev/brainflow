#pragma once
using namespace System;
using namespace BLE_Lib;


namespace Wrapper
{
    public
    ref class BrainAliveLibWrapper
    {
        BrainAliveLibWrapper(void)
        {
            brainalive_obj = gcnew Brainalive();
        }

    public:
        Brainalive^ brainalive_obj;
        static BrainAliveLibWrapper^ instance = gcnew BrainAliveLibWrapper();
    };
}
