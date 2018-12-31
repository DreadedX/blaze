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

    public BootstrapViewModel(@NonNull Application application) {
        super(application);
    }

    public void start() {
        started = true;
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
