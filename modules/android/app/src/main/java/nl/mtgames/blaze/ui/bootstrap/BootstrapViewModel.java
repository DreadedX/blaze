package nl.mtgames.blaze.ui.bootstrap;

import android.app.Application;
import android.arch.lifecycle.AndroidViewModel;
import android.arch.lifecycle.MutableLiveData;
import android.os.AsyncTask;
import android.support.annotation.NonNull;
import android.util.Log;

import java.util.Objects;

public class BootstrapViewModel extends AndroidViewModel {
    private MutableLiveData<String> log;
    private String logString = "";
    private boolean started = false;

    static {
        System.loadLibrary("game");
    }

    private native void startNative();

    public BootstrapViewModel(@NonNull Application application) {
        super(application);
    }

    public void start() {
        if (!started) {
            Log.d("BlazeBootstrap", Objects.requireNonNull(getApplication().getExternalFilesDir(null)).getAbsolutePath());
            Log.d("BlazeBootstrap", "Starting engine");
            // We are running the engine in a background thread
            // Could change in the future if we can render our own loading screen
            Thread thread = new Thread(this::startNative);
            thread.start();
            started = true;
        }
    }

    public boolean isStarted() {
        return started;
    }

    public MutableLiveData<String> getLog() {
        if (this.log == null) {
            this.log = new MutableLiveData<String>();
            this.log.setValue("");
        }
        return this.log;
    }

    public void appendLog(String message) {
        logString += message;
        Log.d("BlazeNative", message);
        getLog().postValue(logString);
    }
}
