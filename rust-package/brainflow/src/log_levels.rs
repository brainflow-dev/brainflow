#[derive(Debug, Copy, Clone, Hash, PartialEq, Eq, FromPrimitive)]
pub enum LogLevels {
    LevelTrace = 0,
    #[doc = " TRACE"]
    LevelDebug = 1,
    #[doc = " DEBUG"]
    LevelInfo = 2,
    #[doc = " INFO"]
    LevelWarn = 3,
    #[doc = " WARN"]
    LevelError = 4,
    #[doc = " ERROR"]
    LevelCritical = 5,
    #[doc = " CRITICAL"]
    LevelOff = 6,
}