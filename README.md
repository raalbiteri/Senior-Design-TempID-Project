# Senior-Design-TempID-Project
Work in Progress Application: Temperature Identification Device that monitors user's temperature and displays to others as a form of contact tracing

Basic Startup Instructions for SE 4910-11:
* User will be presented with the login screen. Still in progress, so do not enter in
    real information. Any username/email will work and any password over 7 characters long will work,
    no verification necessary.
* Once at the Bluetooth scanner page, the app will request permission to turn on Bluetooth and access
    to location. These dialog messages will continue to pop up until permission is granted.
* After permission is granted, turn location on using the notification center or in settings.
* Proceed to turn on TempI.D device if not already on and wait for blue LED to flash indicating that
    it is now discoverable
* Bring TempI.D device within 6 inch distance and press the "Scan" button
* Select the device, usually labeled "TempID0xx" where 'xx' are numbers
* After connecting, blue LED will stop flashing and become solid blue
* Also, user will be taken to Temperature data page where incoming temperatures will be graphed, logged, and
    monitored after checking the "Subscribe" radio button on the top left corner.

Personal Progress made on files: MainActivity.kt, TempDisplay.kt, TempIDStart.ino

MainActivity.kt: Main bluetooth scanner meant to discover the BlueFruit compatible device (e.g TempID00X)/(See example image)
https://drive.google.com/file/d/1Sl5MljbYzxgSk6NKEtE_rvYIA1PMia-c/view?usp=sharing

TempDisplay.kt: After connecting device user can subscribe to temperature/battery notifications to display on graph and log (See example image)

https://drive.google.com/file/d/1Sl5MljbYzxgSk6NKEtE_rvYIA1PMia-c/view?usp=sharing

TempIDStart.ino: Displays temperature taken from temperature sensors on LCD and sends data over Bluetooth to Android app (See example setup)

https://drive.google.com/file/d/1PF3BqqhQ89prgv7pUqRYRUOuvVIARfjf/view?usp=sharing

https://drive.google.com/file/d/1Zr-LfLh-YRi5bRtmqHU0CsQreeOE02Mg/view?usp=sharing

Credit to Punchthrough for remaining .kt files: https://punchthrough.com/android-ble-guide/#Service-discovery-as-part-of-the-connection-flow

Credit to Team0x0E for TempIDStart file: Jacob Schultz (Temperature Sensors), Brycen Hakes (LCD), Raunel Albiter (Bluetooth,NeoPixel,ADC)
