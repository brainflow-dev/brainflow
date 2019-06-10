#pragma once

using namespace System;
using namespace GanglionLib;

namespace Wrapper
{
    public ref class GanglionLibWrapper
    {
        GanglionLibWrapper (void)
        {
            ganglion_obj = gcnew Ganglion ();
        }
    public:
        Ganglion ^ganglion_obj;
        static GanglionLibWrapper ^instance = gcnew GanglionLibWrapper ();
    };
}