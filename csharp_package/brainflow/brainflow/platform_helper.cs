using System;


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
                switch (Environment.OSVersion.Platform)
                {
                    case PlatformID.MacOSX:
                        library_env = LibraryEnvironment.MacOS;
                        Console.Error.Write ("MacOS platform detected");
                        break;
                    case PlatformID.Unix:
                        library_env = LibraryEnvironment.Linux;
                        Console.Error.Write ("Linux platform detected");
                        break;
                    default:
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
                        break;
                }
            }

            return library_env;
        }

        private static LibraryEnvironment library_env = LibraryEnvironment.Unknown;
    }
}

