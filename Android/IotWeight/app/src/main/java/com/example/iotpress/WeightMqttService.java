package com.example.iotpress;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Intent;
import android.os.Build;
import android.os.IBinder;
import android.util.Log;
import android.widget.Toast;

import androidx.annotation.Nullable;
import androidx.core.app.NotificationCompat;
import androidx.core.app.NotificationManagerCompat;

import com.somsakelect.android.mqtt.MqttAndroidClient;

import org.eclipse.paho.client.mqttv3.IMqttActionListener;
import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.IMqttToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;

public class WeightMqttService extends Service {
    public WeightMqttService() {
    }
    String MQTTHOST = "tcp://test.mosquitto.org:1883";
    String subTopic = "WEIGHT_221126/WEIGHT";
    MqttAndroidClient client;


    @Nullable
    @Override
    public IBinder onBind(Intent intent)
    {
        return null;
    }

    @Override
    public void onCreate()
    {
        super.onCreate();
        startForeground(1,new Notification()); // 추가
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        setHost();
        connect();
        client.setCallback(new MqttCallback() { //setCallback for message arrive
            @Override
            public void connectionLost(Throwable cause) {
            }

            @Override
            public void messageArrived(String topic, MqttMessage message) throws Exception {
                String text = new String(message.toString());
                Log.d("valueOf", String.valueOf(message.getPayload()));
                Log.d("text", text);

                if(text.equals("hello"))
                {
                    Intent intent = new Intent(WeightMqttService.this, MainActivity.class);
                    PendingIntent pendingIntent = PendingIntent.getActivity(WeightMqttService.this, 0, intent,PendingIntent.FLAG_UPDATE_CURRENT);

                    if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.O)
                    {
                        NotificationChannel channel = new NotificationChannel("notify test", "notiName", NotificationManager.IMPORTANCE_DEFAULT);
                        NotificationManager manager = getSystemService(NotificationManager.class);
                        manager.createNotificationChannel(channel);
                    }

                    NotificationCompat.Builder builder = new NotificationCompat.Builder(WeightMqttService.this, "notify test");
                    builder.setContentTitle("알림 테스트")
                            .setContentText("이것은 알림 테스트입니다.")
                            .setSmallIcon(R.mipmap.ic_launcher)
                            .setDefaults(Notification.DEFAULT_VIBRATE)
                            .setContentIntent(pendingIntent)
                            .setAutoCancel(true);

                    NotificationManagerCompat managerCompat = NotificationManagerCompat.from(WeightMqttService.this);
                    managerCompat.notify(0,builder.build());

                    Toast.makeText(WeightMqttService.this, "알림 생성", Toast.LENGTH_SHORT).show();
                }
            }

            @Override
            public void deliveryComplete(IMqttDeliveryToken token) {

            }
        });

        return START_REDELIVER_INTENT;
    }

    @Override
    public void onDestroy() {
        disconnect();
        super.onDestroy();
        stopForeground(true);
    }

    private void setHost()
    {
        String clientId = MqttClient.generateClientId();
        client = new MqttAndroidClient(this.getApplicationContext(), MQTTHOST, clientId);
    }

    private void setSubscription()
    {
        try {
            client.subscribe(subTopic, 0);
            Toast.makeText(WeightMqttService.this, "subscribe "+ subTopic, Toast.LENGTH_SHORT).show();
        } catch (MqttException e) {
            e.printStackTrace();
        }
    }

    private void connect()
    {
        Log.d("log", "try connection to "+MQTTHOST);
        try {
            IMqttToken token = client.connect();
            token.setActionCallback(new IMqttActionListener()
            {
                @Override
                public void onSuccess(IMqttToken asyncActionToken)
                {
                    Toast.makeText(WeightMqttService.this, "connected", Toast.LENGTH_SHORT).show();
                    setSubscription();
                }

                @Override
                public void onFailure(IMqttToken asyncActionToken, Throwable exception) {
                    Toast.makeText(WeightMqttService.this, "failed to connect", Toast.LENGTH_SHORT).show();
                    Log.d("log", "failed to connect");
                }
            });
        }
        catch (MqttException e)
        {
            e.printStackTrace();
        }
    }
    private void disconnect()
    {
        try
        {
            IMqttToken token = client.disconnect();
            token.setActionCallback(new IMqttActionListener()
            {
                @Override
                public void onSuccess(IMqttToken asyncActionToken)
                {
                    Toast.makeText(WeightMqttService.this, "disconnected", Toast.LENGTH_SHORT).show();
                }

                @Override
                public void onFailure(IMqttToken asyncActionToken, Throwable exception)
                {
                    Toast.makeText(WeightMqttService.this, "could not disconnect..", Toast.LENGTH_SHORT).show();
                }
            });
        }
        catch (MqttException e)
        {
            e.printStackTrace();
        }
    }
}