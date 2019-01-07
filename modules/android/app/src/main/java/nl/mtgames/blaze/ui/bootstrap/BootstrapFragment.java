package nl.mtgames.blaze.ui.bootstrap;

import android.app.NativeActivity;
import android.app.ProgressDialog;
import android.arch.lifecycle.Observer;
import android.arch.lifecycle.ViewModelProviders;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Environment;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.URL;
import java.net.URLConnection;
import java.util.Objects;

import nl.mtgames.blaze.Native;
import nl.mtgames.blaze.R;

import static android.view.View.*;

public class BootstrapFragment extends Fragment {

    private BootstrapViewModel mViewModel;

    public static BootstrapFragment newInstance() {
        return new BootstrapFragment();
    }

    @Nullable
    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, @Nullable ViewGroup container,
                             @Nullable Bundle savedInstanceState) {
        return inflater.inflate(R.layout.bootstrap_fragment, container, false);
    }

    @Override
    public void onActivityCreated(@Nullable Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        mViewModel = ViewModelProviders.of(this).get(BootstrapViewModel.class);

        final Observer<String> logObserver = log -> {
            TextView logView = Objects.requireNonNull(getView()).findViewById(R.id.message);
            logView.setText(log);
        };
        mViewModel.getLog().observe(this, logObserver);

        final Button downloadButton = Objects.requireNonNull(getView()).findViewById(R.id.download);
        downloadButton.setOnClickListener(v -> {
            new DownloadFile().execute("http://zeus:3000/static/base.flm", "base.flm");
            new DownloadFile().execute("http://zeus:3000/static/my_first_mod.flm", "my_first_mod.flm");
//            new DownloadFile().execute("http://192.168.178.60:3000/static/base.flm", "base.flm");
//            new DownloadFile().execute("http://192.168.178.60:3000/static/my_first_mod.flm", "my_first_mod.flm");
        });
        final Button startButton = Objects.requireNonNull(getView()).findViewById(R.id.start);
        startButton.setOnClickListener(v -> {
            v.setSystemUiVisibility(SYSTEM_UI_FLAG_IMMERSIVE_STICKY | SYSTEM_UI_FLAG_FULLSCREEN | SYSTEM_UI_FLAG_HIDE_NAVIGATION);
            Intent intent = new Intent(getContext(), Native.class);
            startActivity(intent);
            mViewModel.start();
            startState(true);
        });

        startState(mViewModel.isStarted());
    }

    public void startState(boolean started) {
        final Button startButton = Objects.requireNonNull(getView()).findViewById(R.id.start);
        startButton.setEnabled(!started);
    }

    // @todo We need to move this into BootstrapViewModel, that way we can run it from native code
    private class DownloadFile extends AsyncTask<String, Integer, String> {
        private ProgressDialog progressDialog;
        private String fileName;
        private String folder;
        private boolean isDownloaded;

        @Override
        protected void onPreExecute() {
            super.onPreExecute();
            this.progressDialog = new ProgressDialog(BootstrapFragment.this.getContext());
            this.progressDialog.setProgressStyle(ProgressDialog.STYLE_HORIZONTAL);
            this.progressDialog.setCancelable(false);
            this.progressDialog.show();
        }

        @Override
        protected String doInBackground(String... f_url) {
            int count;
            try {
                URL url = new URL(f_url[0]);
                URLConnection connection = url.openConnection();
                connection.connect();
                int lengthOfFile = connection.getContentLength();

                InputStream input = new BufferedInputStream(url.openStream(), 8192);
                fileName = f_url[1];
                folder = Objects.requireNonNull(Objects.requireNonNull(getContext()).getExternalFilesDir(null)).getAbsolutePath();

                File directory = new File(folder);

                // @todo Pretty sure we do not have to do this
                if (!directory.exists()) {
                    directory.mkdir();
                }

                OutputStream output = new FileOutputStream(folder + File.separator + fileName);

                byte data[] = new byte[1024];

                long total = 0;

                while ((count = input.read(data)) != -1) {
                    total += count;
                    publishProgress((int) ((total * 100) / lengthOfFile));
                    Log.d("BlazeBootstrap", "Progress: " + (int)((total * 100) / lengthOfFile));

                    output.write(data, 0, count);
                }

                output.flush();

                output.close();
                input.close();

                return  "Downloaded at: " + folder + File.separator + fileName;
            } catch (Exception e) {
                Log.e("BlazeBootrap", e.getMessage());
            }

            return "Something went wrong";
        }

        @Override
        protected void onProgressUpdate(Integer... progress) {
            progressDialog.setProgress(progress[0]);
        }

        @Override
        protected void onPostExecute(String message) {
            this.progressDialog.dismiss();
            Toast.makeText(getContext(), message, Toast.LENGTH_LONG).show();
        }
    }
}
