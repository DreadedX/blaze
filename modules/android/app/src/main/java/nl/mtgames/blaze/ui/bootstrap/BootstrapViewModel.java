package nl.mtgames.blaze.ui.bootstrap;

import android.app.Application;
import androidx.lifecycle.AndroidViewModel;
import androidx.lifecycle.MutableLiveData;
import androidx.annotation.NonNull;
import android.util.Log;

public class BootstrapViewModel extends AndroidViewModel {
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
}
