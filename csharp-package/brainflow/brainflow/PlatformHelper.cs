using System;

namespace brainflow
{
    public enum LibraryEnvironment
    {
        Unknown,
        Linux,
        x86,
        x64,
    }

    public static class PlatformHelper
    {
        public static LibraryEnvironment GetLibraryEnvironment()
        {
            if (_LibraryEnvironment == LibraryEnvironment.Unknown)
            {
                switch (Environment.OSVersion.Platform)
                {
                    case PlatformID.Unix:
                    case PlatformID.MacOSX:
                        _LibraryEnvironment = LibraryEnvironment.Linux;
                        break;

                    default:
                        {
                            if (Environment.Is64BitProcess)
                                _LibraryEnvironment = LibraryEnvironment.x64;
                            else
                                _LibraryEnvironment = LibraryEnvironment.x86;
                        }
                        break;
                }
            }

            return _LibraryEnvironment;
        }

        private static LibraryEnvironment _LibraryEnvironment = LibraryEnvironment.Unknown;
    }
}

