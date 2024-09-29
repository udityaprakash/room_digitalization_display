import 'package:flutter/material.dart';
import 'package:http/http.dart' as http;

void main() => runApp(MyApp());

class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'OLED Display Controller',
      theme: ThemeData(
        primarySwatch: Colors.blue,
      ),
      home: MyHomePage(),
    );
  }
}

class MyHomePage extends StatefulWidget {
  @override
  _MyHomePageState createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage> {
  TextEditingController textController = TextEditingController();
  int textSize = 1;
  bool isScrolling = false;
  bool showClock = false;  // New variable for the clock toggle

  // Function to send text or clock mode to NodeMCU
  Future<void> sendTextToOLED() async {
    String text = textController.text;
    String url = 'http://192.168.1.65/?';

    if (showClock) {
      url += 'clock=true';
    } else {
      url += 'text=$text&size=$textSize&scroll=$isScrolling&clock=false';
    }

    try {
      final response = await http.get(Uri.parse(url));
      if (response.statusCode == 200) {
        print('Request sent successfully');
      } else {
        print('Failed to send request');
      }
    } catch (e) {
      print('Error: $e');
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text('OLED Display Controller'),
      ),
      body: Padding(
        padding: EdgeInsets.all(16.0),
        child: Column(
          children: <Widget>[
            TextField(
              controller: textController,
              decoration: InputDecoration(labelText: 'Enter text'),
            ),
            Row(
              children: [
                Text('Text Size:'),
                Slider(
                  value: textSize.toDouble(),
                  min: 1,
                  max: 3,
                  divisions: 2,
                  label: textSize.toString(),
                  onChanged: (value) {
                    setState(() {
                      textSize = value.toInt();
                    });
                  },
                ),
              ],
            ),
            Row(
              children: [
                Text('Scrolling:'),
                Switch(
                  value: isScrolling,
                  onChanged: (value) {
                    setState(() {
                      isScrolling = value;
                    });
                  },
                ),
              ],
            ),
            Row(
              children: [
                Text('Show Full-Screen Clock:'),
                Switch(
                  value: showClock,
                  onChanged: (value) {
                    setState(() {
                      showClock = value;
                    });
                  },
                ),
              ],
            ),
            ElevatedButton(
              onPressed: sendTextToOLED,
              child: Text('Send to Display'),
            ),
          ],
        ),
      ),
    );
  }
}
