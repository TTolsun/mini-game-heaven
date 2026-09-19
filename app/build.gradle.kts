import java.util.Properties

plugins {
    alias(libs.plugins.android.application)
}

val releaseProperties = Properties().apply {
    val signingFile = rootProject.file("keystore.properties")
    if (signingFile.isFile) signingFile.inputStream().use { load(it) }
}
fun signingValue(property: String, environment: String): String? =
    System.getenv(environment)?.takeIf { it.isNotBlank() }
        ?: releaseProperties.getProperty(property)?.takeIf { it.isNotBlank() }
val releaseStore = signingValue("storeFile", "RELEASE_STORE_FILE")
val releaseStorePassword = signingValue("storePassword", "RELEASE_STORE_PASSWORD")
val releaseAlias = signingValue("keyAlias", "RELEASE_KEY_ALIAS")
val releaseKeyPassword = signingValue("keyPassword", "RELEASE_KEY_PASSWORD")
val signingValues = listOf(releaseStore, releaseStorePassword, releaseAlias, releaseKeyPassword)
require(signingValues.all { it == null } || signingValues.all { it != null }) {
    "Release signing configuration is incomplete"
}

android {
    namespace = "com.ttolsun.minigameheaven"
    compileSdk = 36
    ndkVersion = "29.0.14206865"

    defaultConfig {
        applicationId = "com.ttolsun.minigameheaven"
        minSdk = 28
        targetSdk = 36
        versionCode = 4
        versionName = "0.3.0"

        ndk {
            abiFilters += listOf("arm64-v8a", "x86_64")
        }

        externalNativeBuild {
            cmake {
                arguments += listOf(
                    "-DANDROID_STL=c++_static",
                    "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON",
                )
                cppFlags += "-std=c++20"
            }
        }
    }

    externalNativeBuild {
        cmake {
            path = file("../native/CMakeLists.txt")
            version = "3.31.6"
        }
    }

    buildFeatures {
        prefab = true
        buildConfig = false
    }

    signingConfigs {
        if (releaseStore != null) {
            create("release") {
                storeFile = rootProject.file(releaseStore)
                storePassword = releaseStorePassword
                keyAlias = releaseAlias
                keyPassword = releaseKeyPassword
            }
        }
    }

    buildTypes {
        release {
            signingConfig = signingConfigs.findByName("release")
            isMinifyEnabled = false
            proguardFiles(getDefaultProguardFile("proguard-android-optimize.txt"), "proguard-rules.pro")
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }
}

dependencies {
    implementation(libs.androidx.games.activity)
    implementation(libs.androidx.appcompat)
}
