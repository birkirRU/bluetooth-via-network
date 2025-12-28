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

capture.DataAvailable += (s, a) =>
{
    byte[] audioData= a.Buffer;

    client.Send(audioData, audioData.Length, ipEndpoint);

};

capture.RecordingStopped += (s, a) =>
{
    capture.Dispose();
};


void SetTimer()
{
    System.Timers.Timer aTimer = new System.Timers.Timer(2000);
    aTimer.Elapsed += (source, e) =>
    {
        capture.StopRecording();
        aTimer.Stop();
    };
    aTimer.Enabled = true;

};


capture.StartRecording();
SetTimer();
while (capture.CaptureState != NAudio.CoreAudioApi.CaptureState.Stopped)
{
    Thread.Sleep(500);
}


