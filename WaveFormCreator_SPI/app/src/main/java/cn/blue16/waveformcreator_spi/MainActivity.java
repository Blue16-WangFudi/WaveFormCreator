package cn.blue16.waveformcreator_spi;

import android.Manifest;
import android.app.AlertDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import java.io.IOException;
import java.io.OutputStream;
import java.util.Set;
import java.util.UUID;

public class MainActivity extends AppCompatActivity {

    private static final int PERMISSION_REQUEST_CODE = 1;
    private BluetoothAdapter bluetoothAdapter;
    private BluetoothSocket bluetoothSocket;
    private OutputStream outputStream;

    private Spinner spinnerWaveform;
    private EditText editFrequency;
    private EditText editAmplitude;
    private EditText editPhase;
    private Button btnSend;

    private final String DEVICE_NAME = "JDY-31-SPP";
    private final UUID MY_UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        TextView title = findViewById(R.id.title);
        spinnerWaveform = findViewById(R.id.spinner_waveform);
        editFrequency = findViewById(R.id.edit_frequency);
        editAmplitude = findViewById(R.id.edit_amplitude);
        editPhase = findViewById(R.id.edit_phase);
        btnSend = findViewById(R.id.btn_send);

        ArrayAdapter<CharSequence> adapter = ArrayAdapter.createFromResource(this,
                R.array.waveforms_array, android.R.layout.simple_spinner_item);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        spinnerWaveform.setAdapter(adapter);

        // 请求蓝牙权限
        requestBluetoothPermissions();

        btnSend.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                sendData();
            }
        });
    }

    private void requestBluetoothPermissions() {
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH) != PackageManager.PERMISSION_GRANTED ||
                ContextCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_ADMIN) != PackageManager.PERMISSION_GRANTED ||
                ContextCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED ||
                ContextCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_SCAN) != PackageManager.PERMISSION_GRANTED) {

            ActivityCompat.requestPermissions(this,
                    new String[]{
                            Manifest.permission.BLUETOOTH,
                            Manifest.permission.BLUETOOTH_ADMIN,
                            Manifest.permission.BLUETOOTH_CONNECT,
                            Manifest.permission.BLUETOOTH_SCAN
                    }, PERMISSION_REQUEST_CODE);
        } else {
            connectBluetooth();
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);

        if (requestCode == PERMISSION_REQUEST_CODE) {
            if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                connectBluetooth();
            } else {
                Toast.makeText(this, "未授予蓝牙权限", Toast.LENGTH_LONG).show();
            }
        }
    }

    private void connectBluetooth() {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setMessage("等待设备连接...")
                .setCancelable(false);

        final AlertDialog alert = builder.create();
        alert.show();

        bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        if (bluetoothAdapter == null) {
            Toast.makeText(this, "此设备不支持蓝牙", Toast.LENGTH_LONG).show();
            finish();
            return;
        }

        if (!bluetoothAdapter.isEnabled()) {
            if (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
                // TODO: Consider calling
                //    ActivityCompat#requestPermissions
                // here to request the missing permissions, and then overriding
                //   public void onRequestPermissionsResult(int requestCode, String[] permissions,
                //                                          int[] grantResults)
                // to handle the case where the user grants the permission. See the documentation
                // for ActivityCompat#requestPermissions for more details.
                return;
            }
            bluetoothAdapter.enable();
        }

        new Thread(new Runnable() {
            @Override
            public void run() {

                Set<BluetoothDevice> pairedDevices = bluetoothAdapter.getBondedDevices();
                for (BluetoothDevice device : pairedDevices) {
                    if (DEVICE_NAME.equals(device.getName())) {
                        try {
                            bluetoothSocket = device.createRfcommSocketToServiceRecord(MY_UUID);
                            bluetoothSocket.connect();
                            outputStream = bluetoothSocket.getOutputStream();
                            runOnUiThread(new Runnable() {
                                @Override
                                public void run() {
                                    alert.dismiss();
                                    Toast.makeText(MainActivity.this, "函数信号发生器连接成功", Toast.LENGTH_SHORT).show();
                                }
                            });
                        } catch (IOException e) {
                            runOnUiThread(new Runnable() {
                                @Override
                                public void run() {
                                    alert.dismiss();
                                    Toast.makeText(MainActivity.this, "函数信号发生器连接失败", Toast.LENGTH_SHORT).show();
                                }
                            });
                            e.printStackTrace();
                        }
                        return;
                    }
                }

                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        alert.dismiss();
                        Toast.makeText(MainActivity.this, "未找到函数信号发生器", Toast.LENGTH_SHORT).show();
                    }
                });
            }
        }).start();
    }

    private void sendData() {
        String waveform = String.valueOf(spinnerWaveform.getSelectedItemPosition());
        String frequency = String.format("%07d", Integer.parseInt(editFrequency.getText().toString()));
        String amplitude = String.format("%04d", Integer.parseInt(editAmplitude.getText().toString()));
        String phase = String.format("%03d", Integer.parseInt(editPhase.getText().toString()));
        String command = "CMD" + waveform + frequency + amplitude + phase;

        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setMessage("正在下发数据...")
                .setCancelable(false);

        final AlertDialog alert = builder.create();
        alert.show();

        new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    outputStream.write(command.getBytes());
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            alert.dismiss();
                            Toast.makeText(MainActivity.this, "数据下发成功", Toast.LENGTH_SHORT).show();
                        }
                    });
                } catch (IOException e) {
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            alert.dismiss();
                            Toast.makeText(MainActivity.this, "数据下发失败", Toast.LENGTH_SHORT).show();
                        }
                    });
                    e.printStackTrace();
                }
            }
        }).start();
    }
}
