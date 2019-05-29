package nl.mtgames.blaze.ui.bootstrap;

import android.app.ProgressDialog;
import androidx.lifecycle.ViewModelProviders;
import android.content.Intent;
import android.os.Bundle;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;

import android.os.Environment;
import android.os.Handler;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.LinkedList;
import java.util.List;
import java.util.Objects;

import nl.mtgames.blaze.Native;
import nl.mtgames.blaze.R;
import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.Response;

public class BootstrapFragment extends Fragment {

    private BootstrapViewModel mViewModel;
    private ProgressDialog progressDialog;

    private Handler handler;

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

        progressDialog = new ProgressDialog(getContext());
        progressDialog.setTitle("Downloading...");
        progressDialog.setMax(100);
        progressDialog.setCancelable(false);
        progressDialog.setProgressStyle(ProgressDialog.STYLE_HORIZONTAL);

        handler = new Handler();

        final Button downloadButton = Objects.requireNonNull(getView()).findViewById(R.id.download);
        downloadButton.setOnClickListener(v -> {
            Thread t = new Thread(download("base.flm", "my_first_mod.flm"));
            t.start();
        });

        final Button startButton = Objects.requireNonNull(getView()).findViewById(R.id.start);
        startButton.setOnClickListener(v -> {
            Intent intent = new Intent(getContext(), Native.class);
            startActivity(intent);
            mViewModel.start();
            startState(true);
        });

        startState(mViewModel.isStarted());
    }

    private void startState(boolean started) {
        final Button startButton = Objects.requireNonNull(getView()).findViewById(R.id.start);
        startButton.setEnabled(!started);
    }

    private Runnable download(String... names) {
        List<String> nameList = new LinkedList<>(Arrays.asList(names));
        return download(nameList);
    }

    private Runnable download(List<String> names) {
        String name = names.get(0);
        names.remove(0);
        return () -> {
            Log.d("BlazeBootstrap", "Downloading: " + name);
            OkHttpClient client = new OkHttpClient();
            Request request = new Request.Builder().url("http://zeus:3000/static/" + name).build();
            try {
                Response response = client.newCall(request).execute();

                float size = Objects.requireNonNull(response.body()).contentLength();
                BufferedInputStream inputStream = new BufferedInputStream(Objects.requireNonNull(response.body()).byteStream());

                OutputStream stream = new FileOutputStream(new File(Objects.requireNonNull(getContext()).getExternalFilesDir(null), name));

                byte[] data = new byte[8129];
                float total = 0;
                int readBytes = 0;

                handler.post(() -> {
                    progressDialog.show();
                    progressDialog.setTitle("Downloading: " + name);
                });

                while ((readBytes = inputStream.read(data)) != -1) {
                    total += readBytes;
                    stream.write(data, 0, readBytes);
                    progressDialog.setProgress((int)(total / size * 100));
                }

                progressDialog.dismiss();
                stream.flush();
                stream.close();

                Objects.requireNonNull(response.body()).close();

                if (names.size() > 0) {
                    handler.post(() -> {
                        Thread t = new Thread(download(names));
                        t.start();
                    });
                }

            } catch (IOException e) {
                e.printStackTrace();
            }
        };
    }
}
