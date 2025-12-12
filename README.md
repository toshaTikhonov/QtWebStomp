# QtWebStomp
A C++ implementation of the Stomp protocol through websocket

## How to build

### Requirements
- Qt5 or Qt6 with WebSockets module
- CMake 3.16 or higher (for CMake build)
- C++11 compatible compiler

### Using CMake (Recommended)

The project supports both Qt5 and Qt6. CMake will automatically detect and use the available Qt version (Qt6 is preferred if both are installed).

#### Linux/macOS
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

#### Windows
```bash
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019"  # Or your Visual Studio version
cmake --build . --config Release
```

To force a specific Qt version:
```bash
cmake .. -DQT_VERSION_MAJOR=5  # Force Qt5
# or
cmake .. -DQT_VERSION_MAJOR=6  # Force Qt6
```

To build without the test project:
```bash
cmake .. -DBUILD_TESTS=OFF
cmake --build .
```

To install the library:
```bash
cmake --build . --target install
```

### Using Visual Studio (Windows only)
Open visual studio, click build. As of now, only debug creates the build/bin and build/include structure. In release you have to create it yourself (it's just copy-pasting).

## How to use

Include the header in your source.
Just do an #include "QTWebStompClientDll.h" . Really, that’s all you need.
 
Create an instance of the client.
An example:
auto myClient = new QTWebStompClient("ws://10.12.4.142:15674/ws", "ugs", "ugs", onConnect, false);
Once the client is created, it will automagically connect. The constructor takes the URL of webstomp as first parameter, second is login, third is passcode. The fourth is a pointer to a function that will execute as soon as the client is connected aka the on connect callback (made it like the JS client that Andrew was working with for simplicity). You can set the last bool to true to see debug messages in the console.
 
Subscribe to a queue

On the onConnect method, you can subscribe to a Queue like this:
 
void onConnect() {
myClient->Subscribe("/queue/agent", onMessage);
}
 
The Subscribe method takes two mandatory parameters: the first one is the suburl you want to subscribe to (in this case the agent queue). The second one is a pointer to a function you want to execute when you receive a message (just like the JS client again). There’s a third optional parameter: the way you want to ack the messages (takes an enum). It defaults to auto (no manual ack). If you use the other options, you can ack messages with the ack method that the client contains. See example below.
 
Execute something when you receive a message

The onMessage function will be receiving a StompMessage object by constant reference. You can keep a copy of the object if you want, but then it’ll be your job to clean up afterwards =). If you don’t keep a copy there are no memleaks, I checked. (What is this, I have to manage my memory now?!? Welcome to C++!)
So an example of printing the message and acking it would be like this:
 
void onMessage(const StompMessage &s) {
              qDebug() << "The message we got is\r\n" << s.toString().c_str(); // The toString function shows the message in a very readable way.
              myClient->Ack(s); // You don’t need to ack the message if you use automode. You can either specify the entire message as a parameter of ack or just the id you wanna ack if you wanna ack it later but not save the entire object.
}

That’s it! You can access the individual headers with the s.m_headers vector and the message with s.m_message. 

## Known-bugs
None! (Yet)

## SSL Support
To get ssl to work in iOS, you have to do nothing. Just make sure that the url is wss:// not ws:// . In windows you'll need a set of qt libraries that have been compiled with openssl support. Then add the ddls to a system directory or the executable folder.


