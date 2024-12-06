import 'package:flutter/material.dart';
import 'package:flutter_blue/flutter_blue.dart';

void main() {
  runApp(ControladorOrta());
}

class ControladorOrta extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      debugShowCheckedModeBanner: false,
      home: MenuPrincipal(),
    );
  }
}

class MenuPrincipal extends StatefulWidget {
  @override
  _MenuPrincipalState createState() => _MenuPrincipalState();
}

class _MenuPrincipalState extends State<MenuPrincipal> {
  double temperatura = 25.0;
  double umidade = 60.0;

  FlutterBlue flutterBlue = FlutterBlue.instance;
  BluetoothDevice? connectedDevice;
  BluetoothCharacteristic? characteristic;

  @override
  void initState() {
    super.initState();
    scanForDevices();
  }

  void scanForDevices() {
    flutterBlue.startScan(timeout: Duration(seconds: 4));

    flutterBlue.scanResults.listen((results) {
      for (ScanResult result in results) {
        if (result.device.name == "HC-05") {
          connectToDevice(result.device);
          break;
        }
      }
    });
  }

  void connectToDevice(BluetoothDevice device) async {
    await flutterBlue.stopScan();
    await device.connect();
    setState(() {
      connectedDevice = device;
    });

    // Descubra os serviços e características do dispositivo
    List<BluetoothService> services = await device.discoverServices();
    for (BluetoothService service in services) {
      for (BluetoothCharacteristic c in service.characteristics) {
        if (c.properties.write) {
          characteristic = c;
        }
      }
    }
  }

  void sendCommand(String command) async {
    if (characteristic != null) {
      await characteristic!.write(command.codeUnits);
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text('Controlador de Orta'),
        centerTitle: true,
      ),
      body: connectedDevice == null
          ? Center(child: Text("Conectando ao Bluetooth..."))
          : Column(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                _buildControle("Temperatura", temperatura, "°C", (newValue) {
                  setState(() {
                    temperatura = newValue;
                  });
                  sendCommand("TEMP:${temperatura.toInt()}");
                }),
                SizedBox(height: 20),
                _buildControle("Umidade", umidade, "%", (newValue) {
                  setState(() {
                    umidade = newValue;
                  });
                  sendCommand("UMID:${umidade.toInt()}");
                }),
              ],
            ),
    );
  }

  Widget _buildControle(
      String titulo, double valor, String unidade, ValueChanged<double> onChanged) {
    return Card(
      elevation: 5,
      margin: EdgeInsets.symmetric(horizontal: 20),
      child: Padding(
        padding: const EdgeInsets.all(16.0),
        child: Column(
          children: [
            Text(
              titulo,
              style: TextStyle(fontSize: 18, fontWeight: FontWeight.bold),
            ),
            SizedBox(height: 10),
            Text(
              '${valor.toStringAsFixed(1)} $unidade',
              style: TextStyle(fontSize: 24),
            ),
            Slider(
              value: valor,
              min: (titulo == "Temperatura") ? 0.0 : 0.0,
              max: (titulo == "Temperatura") ? 50.0 : 100.0,
              divisions: 100,
              label: valor.toStringAsFixed(1),
              onChanged: onChanged,
            ),
          ],
        ),
      ),
    );
  }
}
