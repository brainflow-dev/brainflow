using System;
using System.Runtime.InteropServices;

namespace brainflow
{
    public enum LibraryEnvironment
    {
        Unknown,
        MacOS,
        Linux,
        x86,
        x64,
    }

    public static class PlatformHelper
    {
        public static LibraryEnvironment get_library_environment ()
        {
            if (library_env == LibraryEnvironment.Unknown)
            {
                if (RuntimeInformation.IsOSPlatform (OSPlatform.Linux))
                {
                    Console.Error.Write ("Linux platform detected");
                    library_env = LibraryEnvironment.x64;
                }
                if (RuntimeInformation.IsOSPlatform (OSPlatform.OSX))
                {
                    Console.Error.Write ("OSX platform detected");
                    library_env = LibraryEnvironment.MacOS;
                }
                if (RuntimeInformation.IsOSPlatform (OSPlatform.Windows))
                {
                    if (Environment.Is64BitProcess)
                    {
                        Console.Error.Write ("Win64 platform detected");
                        library_env = LibraryEnvironment.x64;
                    }
                    else
                    {
                        Console.Error.Write ("Win32 platform detected");
                        library_env = LibraryEnvironment.x86;
                    }
                }
            }

            return library_env;
        }

        private static LibraryEnvironment library_env = LibraryEnvironment.Unknown;
    }
}

