# This is a configuration file for ProGuard.
# http://proguard.sourceforge.net/index.html#manual/usage.html

# For more details, see
#   http://developer.android.com/guide/developing/tools/proguard.html

# Add any project specific keep options here:

# If your project uses WebView with JS, uncomment the following
# and specify the fully qualified class name to the JavaScript interface
# class:
#-keepclassmembers class fqcn.of.javascript.interface.for.webview {
#   public *;
#}

# Keep C++ JNI methods
-keepclasseswithmembernames class * {
    native <methods>;
}

# Keep SDL Activity
-keep class org.libsdl.app.SDLActivity

# Keep miniCar MainActivity
-keep class com.example.minicar.MainActivity

