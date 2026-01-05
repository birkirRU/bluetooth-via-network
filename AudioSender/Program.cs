// See https://aka.ms/new-console-template for more information
// Console.WriteLine("Hello, World!");



using System.Net;
using System.Net.Sockets;
using NAudio.Wave;

UdpClient client = new UdpClient();

int port = 5000;
IPAddress ipaddress = IPAddress.Parse("192.168.86.32");
IPEndPoint ipEndpoint = new IPEndPoint(ipaddress, port);

var capture = new WasapiLoopbackCapture();


byte[] header = new byte[12];
// Sample rate (4 bytes)
BitConverter.GetBytes(capture.WaveFormat.SampleRate).CopyTo(header, 0);
// Bit depth (4 bytes)  
BitConverter.GetBytes(capture.WaveFormat.BitsPerSample).CopyTo(header, 4);
// Channels (4 bytes)
BitConverter.GetBytes(capture.WaveFormat.Channels).CopyTo(header, 8);

// Send header
client.Send(header, header.Length, ipEndpoint);

capture.DataAvailable += (s, a) =>
{
    byte[] audioData= a.Buffer;
    Console.WriteLine($"Buffer size: {a.BytesRecorded}");

    client.Send(audioData, a.BytesRecorded, ipEndpoint);
};

capture.RecordingStopped += (s, a) =>
{
    capture.Dispose();
};


void SetTimer()
{
    System.Timers.Timer aTimer = new System.Timers.Timer(1000);
    aTimer.Elapsed += (source, e) =>
    {
        capture.StopRecording();
        aTimer.Stop();
    };
    aTimer.Enabled = true;

};


capture.StartRecording();
SetTimer();    
while (capture.CaptureState != NAudio.CoreAudioApi.CaptureState.Stopped) {
    Thread.Sleep(500);
}


