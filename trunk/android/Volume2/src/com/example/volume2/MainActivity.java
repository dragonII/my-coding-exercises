package com.example.volume2;

import android.media.AudioManager;
import android.os.Bundle;
import android.app.Activity;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.support.v4.app.NotificationCompat;
import android.view.Menu;
import android.widget.SeekBar;
import android.widget.TextView;

public class MainActivity extends Activity {

	AudioManager volume_am;
	SeekBar volume_seekBar;
	TextView volume_textView;
	int max_volume;
	int max_progress;
	NotificationManager mNotificationManagner;
	NotificationCompat.Builder mBuilder;
	int mNotificationId;
	PendingIntent mPendingIntent;
	
	private void createMyNotification(int current_volume)
	{
		
		float current_percentage = (float)current_volume / (float)max_volume;
		int current_progress = (int)(current_percentage * max_progress);
		
		// Creates an Intent for the Activity
		Intent notifyIntent = new Intent(this, MainActivity.class);
		
		// Sets the Activity to start in a new, empty task
		notifyIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TASK);
		
		// Create the PendingIntent
		mPendingIntent = PendingIntent.getActivity(this, 0, notifyIntent, PendingIntent.FLAG_UPDATE_CURRENT);
		
		mBuilder.setContentIntent(mPendingIntent);
		
		mBuilder.setContentText(Integer.toString(current_progress) + "%");
		mNotificationManagner.notify(mNotificationId, mBuilder.build()); 
	}
	
	private void intializeAll()
	{
		volume_am = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
		volume_seekBar = (SeekBar) findViewById(R.id.adjust_seekbar);
		volume_textView = (TextView)findViewById(R.id.current_value_text);
		mNotificationManagner =	(NotificationManager)getSystemService(Context.NOTIFICATION_SERVICE);
		
		max_volume = volume_am.getStreamMaxVolume(AudioManager.STREAM_MUSIC);
		max_progress = volume_seekBar.getMax();
		
		mNotificationId = 1;
		mBuilder =
				new NotificationCompat.Builder(this)
				.setSmallIcon(R.drawable.ic_volume)
				.setContentTitle("Volume Adjust");
	}
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		setContentView(R.layout.activity_main);

		intializeAll();
		
		int current_volume = volume_am.getStreamVolume(AudioManager.STREAM_MUSIC);
		setCurrentSeekBarProgressByVolume(current_volume);
		
		createMyNotification(current_volume);
		
		volume_seekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
			
			@Override
			public void onStopTrackingTouch(SeekBar seekBar) {
				// TODO Auto-generated method stub
				
			}
			
			@Override
			public void onStartTrackingTouch(SeekBar seekBar) {
				// TODO Auto-generated method stub
				
			}
			
			@Override
			public void onProgressChanged(SeekBar seekBar, int progress,
					boolean fromUser) {
				// TODO Auto-generated method stub

				seekBar = volume_seekBar;
				int new_progress = seekBar.getProgress();
				setNewSeekBarProgressByProgress(new_progress);
			}
			
			
		});

		//finish();
	}
	
	private int setCurrentSeekBarProgressByVolume(int current_volume)
	{
		float current_percentage = (float)current_volume / (float)max_volume;
		int current_progress = (int)(current_percentage * max_progress);
		volume_seekBar.setProgress(current_progress);
		volume_textView.setText(Integer.toString(current_progress) + "%");
		return current_progress;
	}
	
	private void setNewSeekBarProgressByProgress(int new_progress)
	{
		volume_seekBar.setProgress(new_progress);
		float new_percentage = (float)new_progress / (float)max_progress;
		int new_volume = (int)(new_percentage * max_volume);
		volume_am.setStreamVolume(AudioManager.STREAM_MUSIC, new_volume, 0);
		volume_textView.setText(Integer.toString(new_progress) + "%");
		mBuilder.setContentText(Integer.toString(new_progress) + "%");
		mNotificationManagner.notify(mNotificationId, mBuilder.build());
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}
	
}
