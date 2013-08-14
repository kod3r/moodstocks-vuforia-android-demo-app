# Moodstocks Vuforia - Android demo app

This demo application illustrates the combined use of Moodstocks SDK and Vuforia SDK to boost the capabilities of the Vuforia SDK.

If you haven't done it yet, you should probably read [this article](http://www.moodstocks.com/2013/08/08/improving-qualcomms-vuforia-with-the-moodstocks-instant-image-matching/) on the genesis of this project before going on!

## Features

* This demo uses the Moodstocks SDK and its Android wrapper for image recognition.
* It uses Vuforia SDK for Android to add augmented reality thanks to its powerful image tracking capabilities.
* It currently features two modes: one that displays and tracks the recognized image borders, the other one that overlays the target with a semi-transparent texture displaying its ID. You can easily switch from one to the other using the upper-right "settings" button!

![borders](https://github.com/Moodstocks/moodstocks-vuforia-android-demo-app/wiki/borders.jpg) ![id](https://github.com/Moodstocks/moodstocks-vuforia-android-demo-app/wiki/id.jpg)

## Prerequisites

* An Android development environment (including the Android NDK if you want to compile the code from sources).
* A Moodstocks [developer account](https://developers.moodstocks.com/register) and the latest version of the [Moodstocks SDK for Android](https://developers.moodstocks.com/downloads),
* A Vuforia developer account and the latest version of the [Vuforia SDK for Android](https://developer.vuforia.com/resources/sdk/android). It's important that you follow the [installation steps](https://developer.vuforia.com/resources/dev-guide/step-2-installing-vuforia-sdk) carefully to have a valid development environment!

## Compatibility

The application requires an Android device featuring an ARM CPU, and running Android 4.0 (Ice Cream Sandwich) or more.

## Usage

The C++ libraries used in this project are distributed under two forms:
* pre-compiled libraries to easily try the app,
* source code, if you want to customize this project!

#### Quick start: try the app!

You can test the application on your own images in a few minutes, following these simple steps:
* Clone this repository and open the project in Eclipse
* From the [Vuforia SDK for Android](https://developer.vuforia.com/resources/sdk/android), drag and drop the content of the `build/lib` folder into the `libs` directory of your Eclipse project.
* Download the [Moodstocks SDK for Android](https://developers.moodstocks.com/downloads), drag and drop the `libs` and `src` folders in the Eclipse project.
* Replace your Moodstocks API key/secret pair in [HomeScreen.java](https://github.com/Moodstocks/moodstocks-vuforia-android-demo-app/blob/master/src/com/moodstocks/vuforia/HomeScreen.java#L28). Ensure you have [indexed images to recognize](https://developers.moodstocks.com/doc)!

And that's it: you can run the application and see it in action!

#### Want to tinker with the code?

Now that you have tested the application, you may want to explore and customize the source code. The [wiki](https://github.com/Moodstocks/moodstocks-vuforia-android-demo-app/wiki) will provide you documentation and insights on how the application works, so you do not get lost in the code!

Once modified to your needs, you will have to rebuild the native (C++) part of the code. In a terminal:
* Specify the location of your version of the Vuforia SDK for Android in a `VUFORIA_SDK` variable: `export VUFORIA_SDK=/path/to/vuforia-sdk-android-*-*-*/`,
* Specify the location of your version of the Moodstocks SDK for Android in a `MOODSTOCKS_SDK` variable: `export MOODSTOCKS_SDK=path/to/moodstocks/sdk/v3_6-r2-android-9`,
* From the root directory of the project, run the provided build script: `bash make.sh`.

This will override the prebuilt version of the native libraries provided in this repository with your custom version!

## Troubleshooting

Having any issue with the above instructions?

Ensure you have carefully followed the Vuforia SDK [installation steps](https://developer.vuforia.com/resources/dev-guide/step-2-installing-vuforia-sdk). In particular, ensure your project has
the right dependencies on the `QCAR.jar` package. In your Eclipse project, you should be able to see the following line:

![QCAR.jar](https://github.com/Moodstocks/moodstocks-vuforia-android-demo-app/wiki/QCAR_jar.jpg)

The `libs` folder of your Eclipse project should contain the following elements. If it's not the case, you probably have skipped one of the instructions!

![libs folder](https://github.com/Moodstocks/moodstocks-vuforia-android-demo-app/wiki/libs_folder.jpg)

Still having trouble? Don't hesitate to contact us on our [Help Center](http://help.moodstocks.com/), we'll be glad to help you!
