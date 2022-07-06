package brainflow;

import com.sun.jna.Platform;

import java.io.File;
import java.io.InputStream;
import java.nio.file.Files;
import java.nio.file.Path;

class NativeLoader
{
    static Path unpack_from_jar (String lib_name)
    {
        try
        {
            File file = new File (lib_name);
            if (file.exists ())
                file.delete ();
            InputStream link = (NativeLoader.class.getResourceAsStream (lib_name));
            Files.copy (link, file.getAbsoluteFile ().toPath ());
            return file.getAbsoluteFile ().toPath ();
        } catch (Exception io)
        {
            System.err.println ("file: " + lib_name + " is not found in jar file");
            return null;
        }
    }

    static void copy_to_temp_dir (String lib_name)
    {
        // https://developer.apple.com/library/archive/documentation/FileManagement/Conceptual/FileSystemProgrammingGuide/MacOSXDirectories/MacOSXDirectories.html
        File jnatmp = new File (System.getProperty ("user.home"), "Library/Caches/JNA/temp/" + lib_name);
        try
        {
            if (jnatmp.exists ())
                jnatmp.delete ();
            InputStream link = (NativeLoader.class.getResourceAsStream (lib_name));
            Files.copy (link, jnatmp.getAbsoluteFile ().toPath ());
        } catch (Exception io)
        {
            System.err.println ("file: " + lib_name + " is not found in jar file");
        }
    }
}
