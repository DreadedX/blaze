package nl.mtgames.blaze;

import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;

import nl.mtgames.blaze.ui.bootstrap.BootstrapFragment;

public class Bootstrap extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.bootstrap_activity);
        if (savedInstanceState == null) {
            getSupportFragmentManager().beginTransaction()
                    .replace(R.id.container, BootstrapFragment.newInstance())
                    .commitNow();
        }
    }
}
