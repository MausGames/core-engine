package at.mausgames.app;

import org.libsdl.app.SDLActivity;
import android.os.*;


public class CoreApp extends SDLActivity
{
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
    }

    @Override
    public void onDestroy()
    {
        super.onDestroy();

        // fully kill application to force library reloading
        System.runFinalizersOnExit(true);
        System.exit(0);
    }
}
