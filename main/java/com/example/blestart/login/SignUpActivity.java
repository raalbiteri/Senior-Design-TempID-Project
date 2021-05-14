package com.example.blestart.login;

import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ProgressBar;

import androidx.appcompat.app.AppCompatActivity;

import com.amplifyframework.auth.AuthUserAttributeKey;
import com.amplifyframework.auth.options.AuthSignUpOptions;
import com.amplifyframework.core.Amplify;
import com.example.blestart.R;

public class SignUpActivity extends AppCompatActivity {

    private LoginViewModel loginViewModel;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_sign_up);

//        try {
//            Amplify.addPlugin(new AWSCognitoAuthPlugin());
//            Amplify.configure(getApplicationContext());
//            Log.i("MyAmplifyApp", "Initialized Amplify");
//        } catch (AmplifyException error) {
//            Log.e("MyAmplifyApp", "Could not initialize Amplify", error);
//        }
//
//        Amplify.Auth.fetchAuthSession(
//                result -> Log.i("AmplifyQuickstart", result.toString()),
//                error -> Log.e("AmplifyQuickstart", error.toString())
//        );


        final EditText usernameEditText = findViewById(R.id.username);
        final EditText passwordEditText = findViewById(R.id.password);
        final Button signUpButtonn = findViewById(R.id.signup_button);
        final ProgressBar loadingProgressBar = findViewById(R.id.loading);
        final Button verificationButton = findViewById(R.id.sendVerification);
        final EditText verificationCode = findViewById(R.id.verificationCode);
        verificationButton.setEnabled(true);
        signUpButtonn.setEnabled(true);





        verificationButton.setOnClickListener(new View.OnClickListener(){
            @Override
            public void onClick(View v){
                Amplify.Auth.signUp(
                        usernameEditText.getText().toString(),
                        passwordEditText.getText().toString(),
                        AuthSignUpOptions.builder().userAttribute(AuthUserAttributeKey.email(), usernameEditText.getText().toString()).build(),
                        result -> Log.i("AuthQuickStart", "Result: " + result.toString()),
                        error -> Log.e("AuthQuickStart", "Sign up failed", error)
                );
            }
        });



        signUpButtonn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Amplify.Auth.confirmSignUp(
                        usernameEditText.getText().toString(),
                        verificationCode.getText().toString(),
                        result -> Log.i("AuthQuickstart", result.isSignUpComplete() ? "Sign in succeeded" : "Sign in not complete"),
                        error -> Log.e("AuthQuickstart", error.toString())
                );

                // Add the code to go to mainActivity
                Intent intent = new Intent(SignUpActivity.this, LoginActivity.class);
                startActivity(intent);

            }
        });
    }


}