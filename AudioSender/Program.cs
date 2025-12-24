// See https://aka.ms/new-console-template for more information
// Console.WriteLine("Hello, World!");


using System.Net.Sockets;

UdpClient client = new UdpClient();

string piAddress = "192.168.86.32";

int port = 5000;

string message = "Hello Pi!";

byte[] data = System.Text.Encoding.UTF8.GetBytes(message);

client.Send(data, data.Length, piAddress, port);

Console.WriteLine("Sent!");

