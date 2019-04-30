# iTorrent - iOS Torrent client App

![](https://img.shields.io/badge/iOS-9.3+-blue.svg)
![](https://app.bitrise.io/app/26ce0756a727335c/status.svg?token=BLhjBICoPvmOtO1nzIVMYQ&branch=master)

## Screenshots

![itorrent](https://user-images.githubusercontent.com/9553519/42249216-da6f6190-7f32-11e8-9126-e559be69ebf5.png)

## Info

## This is fork of iTorrent that removes the need for CocoaPods, and removes ads and the donate button
## It also sets the background to trueblack in dark mode, to fix battery life and ugliness issues

All credits to XITRIX; throw him some bones if you like the app.

It is an ordinary torrent client for iOS with Files app support.

What can this app do:
- Download in the background
- Add torrent files from Share menu (Safari and other apps)
- Add magnet links directly from Safari
- Store files in Files app (iOS 11+)
- File sharing directly from app
- Download torrent by link
- Download torrent by magnet
- Send notification on torrent downloaded
- FTP Server (unstable)
- Select files to download or not
- Change UI to dark theme
- ??? 

## Localization

iTorrent supports the following languages:
- English
- Russian

If you are fluent in the languages not listed above and want to help with translation, you are welcome!
Submit a request upstream to the XITRIX repo; I track master as closely as I can.

## Build
Steps:
- clone the repo
- choose if you are building for real device or simulator
- remove armv7 (real) or x64 (simulator) from framework depending on which you want
- Open .xcworkspace, change organization to your preferred identifier
- Build it

### Warning!
This repo contains two iTorrent frameworks. Be sure to rename the correct framework depending on what you are building for.

## Libraries used

- [LibTorrent](https://github.com/arvidn/libtorrent)
- [BackgroundTask](https://github.com/yarodevuci/backgroundTask)
- [Orianne-FTP-Server (My fork)](https://github.com/XITRIX/Orianne-FTP-Server)

## License

Copyright (c) 2018 XITRIX (Vinogradov Daniil)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal 
in the Software without restriction, including without limitation the rights 
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

