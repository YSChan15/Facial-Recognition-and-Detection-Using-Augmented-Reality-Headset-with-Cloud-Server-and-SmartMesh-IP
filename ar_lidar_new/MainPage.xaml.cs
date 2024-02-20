/// <Title>
/// UWP Facial Recognition Program for Microsoft Hololens 2 / Windows Powered Device.
/// This is a part of Senior Design Project Fall 2023 / Spring 2024, 
/// Department of Electrical and Computer Engineering, St Cloud State University.
/// 
/// Title: Facial Recognition and Detection using Augmented Reality Headset with 
///        Cloud Server and SmartMesh IP Network
/// 
/// Advisor: Dr Zheng
/// 
/// Committee: Dr Hossain, Dr Cavalcanti
/// 
/// Group members: Yu Sheng Chan, Brycen Hillukka, Hayden Scott, Brandon Wieberdink 
/// 
/// This project is sponsored by a grant from the Emerson cooperation.
/// </Title>


/// <Introduction>
/// Author - Yu Sheng Chan, Brycen Hillukka 
///
/// Description - this project utilizies Microsoft Hololens 2 to perform tasks and display the following information:
/// Spring 2023 (Junior Undergraduate Research) - Yu Sheng Chan, Brycen Hillukka 
/// 1.  Distance measurement using LiDAR
/// 2.  Three Dimensional coordinates of the pointed distance
///
/// Fall 2023 (Senior Design) - Yu Sheng Chan
/// 3.  GPS location of the user
/// 4.  Angle relative to North
/// 5.  GPS location of pointed distance by Haversine's formula  
/// 6.  Face detection 
/// 7.  Sending pictures to the server using sockets
///
/// Spring 2024 (Senior Design) - Yu Sheng Chan 
/// 8.  Sending cropped faces to the server using sockets if face is detected
/// 9.  Adding feature to connect to / disconnect from the server at any given time
/// 10. Receiving names based on the cropped faces sent to the server for facial recognition
/// 11. Names on each specific face box for facial recognition and detection 
/// </Introduction>


/// <Disclaimer>
/// The following code uses reference from github under microsoft API samples under the MIT License.  
/// Available at: https://github.com/microsoft/windows-universal-samples
/// </Disclaimer>


/// <License>
/*
The MIT License (MIT)

Copyright (c) Microsoft Corporation

Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE. 
*/
/// </License>


// Necessary libaries to include 
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using Windows.ApplicationModel;
using Windows.Devices.Bluetooth;
using Windows.Devices.Bluetooth.Advertisement;
using Windows.Devices.Bluetooth.GenericAttributeProfile;
using Windows.Devices.Enumeration;
using Windows.Devices.Sensors;
using Windows.Foundation;
using Windows.Graphics.Display;
using Windows.Graphics.Imaging;
using Windows.Media;
using Windows.Media.Capture;
using Windows.Media.Core;
using Windows.Media.FaceAnalysis;
using Windows.Media.MediaProperties;
using Windows.Security.Cryptography;
using Windows.Storage;
using Windows.Storage.FileProperties;
using Windows.Storage.Streams;
using Windows.System.Display;
using Windows.UI;
using Windows.UI.Core;
using Windows.UI.ViewManagement;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

// The Blank Page item template is documented at https://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409
namespace ar_lidar_new
{
    public sealed partial class MainPage : Page
    {
        /// <summary>
        /// BLE variables. Added in Spring 2023 (Junior Undergraduate Research). 
        /// </summary>
        #region BLE variables
        GattDeviceService service = null;
        GattCharacteristic charac = null;
        Guid MyService_GUID;
        Guid MYCharacteristic_GUID;
        string bleDevicName = "BT05"; // Device name of BLE Module. 
        long deviceFoundMilis = 0, serviceFoundMilis = 0;
        long connectedMilis = 0, characteristicFoundMilis = 0;
        long WriteDescriptorMilis = 0;
        Stopwatch stopwatch;
        Thread thr;
        static string dataFromNotify = "";
        #endregion

        /// <summary>
        /// Face detection. Added in Summer 2023 (Research).
        /// </summary>
        #region Face detection variables 
        // Receive notifications about rotation of the device and UI and apply any necessary rotation to the preview stream and UI controls.
        private readonly DisplayInformation _displayInformation = DisplayInformation.GetForCurrentView();
        private readonly SimpleOrientationSensor _orientationSensor = SimpleOrientationSensor.GetDefault(); 
        private SimpleOrientation _deviceOrientation = SimpleOrientation.NotRotated;
        private DisplayOrientations _displayOrientation = DisplayOrientations.Portrait;

        // Rotation metadata to apply to the preview stream and recorded videos (MF_MT_VIDEO_ROTATION).
        // Reference: http://msdn.microsoft.com/en-us/library/windows/apps/xaml/hh868174.aspx
        private static readonly Guid RotationKey = new Guid("C380465D-2271-428C-9B83-ECEA3B4A85C1");

        // Folder in which the captures will be stored (initialized in SetupUiAsync).
        private StorageFolder _captureFolder = null;

        // Prevent the screen from sleeping while the camera is running.
        private readonly DisplayRequest _displayRequest = new DisplayRequest();

        // For listening to media property changes.
        private readonly SystemMediaTransportControls _systemMediaControls = SystemMediaTransportControls.GetForCurrentView();

        // MediaCapture and its state variables.
        private MediaCapture _mediaCapture;
        private IMediaEncodingProperties _previewProperties;
        private bool _isInitialized;
        private bool _isRecording;

        // Information about the camera device.
        private bool _mirroringPreview;
        private bool _externalCamera;

        // For facial detection effect.
        private FaceDetectionEffect _faceDetectionEffect;
        #endregion

        public MainPage()
        {
            this.InitializeComponent();
            
            // Preferred size of the UWP program.
            ApplicationView.PreferredLaunchViewSize = new Windows.Foundation.Size(1920, 1080);
            ApplicationView.PreferredLaunchWindowingMode = ApplicationViewWindowingMode.PreferredLaunchViewSize;

            // Stopwatch for the BLE module 
            stopwatch = new Stopwatch();
            // NOTE: Please use any program, such as BLE finder (Link provided on README.md under Bluetooth connection section)
            //       on Android to find your service and characteristics.
            //
            //       The following characteristics and services is only applicable to this project's BLE module on the Arduino measuring system.
            // Service of BLE module. 
            MyService_GUID = new Guid("");
            // Characteristic of BLE module. 
            MYCharacteristic_GUID = new Guid("");
            StartWatching();
            // Start the thread to update textbox.
            thr = new Thread(new ThreadStart(UpdateTextBox));
            thr.Start();
            
            // Do not cache the state of the UI when suspending/navigating.
            NavigationCacheMode = NavigationCacheMode.Disabled;

            // Useful to know when to initialize/clean up the camera.
            Application.Current.Suspending += Application_Suspending;
            Application.Current.Resuming += Application_Resuming;

            // When program starts, do not show disconnect button on screen.
            // Disconnect button will ONLY appear when there's a made connection between server and the program. 
            button_disconnect.Opacity = 0;
            button_disconnect.Visibility = Visibility.Collapsed;
        }

        /// <summary>
        /// Addition of Face Detection in Summer 2023, Referenced from Microsoft Sample Page. 
        /// </summary>
        #region Face Detection Summer 2023 
        private async void Application_Suspending(object sender, SuspendingEventArgs e)
        {
            // Handle global application events only if this page is active
            if (Frame.CurrentSourcePageType == typeof(MainPage))
            {
                var deferral = e.SuspendingOperation.GetDeferral();
                await CleanupCameraAsync();
                await CleanupUiAsync();
                deferral.Complete();
            }
        }

        private async void Application_Resuming(object sender, object o)
        {
            // Handle global application events only if this page is active
            if (Frame.CurrentSourcePageType == typeof(MainPage))
            {
                await SetupUiAsync();
                await InitializeCameraAsync();
            }
        }

        protected override async void OnNavigatedTo(NavigationEventArgs e)
        {
            await SetupUiAsync();
            await InitializeCameraAsync();
        }

        protected override async void OnNavigatingFrom(NavigatingCancelEventArgs e)
        {
            // Handling of this event is included for completenes, as it will only fire when navigating between pages and this sample only includes one page
            await CleanupCameraAsync();
            await CleanupUiAsync();
        }

        /// <summary>
        /// In the event of the app being minimized this method handles media property change events. If the app receives a mute
        /// notification, it is no longer in the foregroud.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="args"></param>
        private async void SystemMediaControls_PropertyChanged(SystemMediaTransportControls sender, SystemMediaTransportControlsPropertyChangedEventArgs args)
        {
            await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, async () =>
            {
                // Only handle this event if this page is currently being displayed
                if (args.Property == SystemMediaTransportControlsProperty.SoundLevel && Frame.CurrentSourcePageType == typeof(MainPage))
                {
                    // Check to see if the app is being muted. If so, it is being minimized.
                    // Otherwise if it is not initialized, it is being brought into focus.
                    if (sender.SoundLevel == SoundLevel.Muted)
                    {
                        await CleanupCameraAsync();
                    }
                    else if (!_isInitialized)
                    {
                        await InitializeCameraAsync();
                    }
                }
            });
        }

        /// <summary>
        /// Occurs each time the simple orientation sensor reports a new sensor reading.
        /// </summary>
        /// <param name="sender">The event source.</param>
        /// <param name="args">The event data.</param>
        private async void OrientationSensor_OrientationChanged(SimpleOrientationSensor sender, SimpleOrientationSensorOrientationChangedEventArgs args)
        {
            if (args.Orientation != SimpleOrientation.Faceup && args.Orientation != SimpleOrientation.Facedown)
            {
                // Only update the current orientation if the device is not parallel to the ground. This allows users to take pictures of documents (FaceUp)
                // or the ceiling (FaceDown) in portrait or landscape, by first holding the device in the desired orientation, and then pointing the camera
                // either up or down, at the desired subject.
                //Note: This assumes that the camera is either facing the same way as the screen, or the opposite way. For devices with cameras mounted
                //      on other panels, this logic should be adjusted.
                _deviceOrientation = args.Orientation;
                await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () => UpdateButtonOrientation());
            }
        }

        /// <summary>
        /// This event will fire when the page is rotated, when the DisplayInformation.AutoRotationPreferences value set in the SetupUiAsync() method cannot be not honored.
        /// </summary>
        /// <param name="sender">The event source.</param>
        /// <param name="args">The event data.</param>
        private async void DisplayInformation_OrientationChanged(DisplayInformation sender, object args)
        {
            _displayOrientation = sender.CurrentOrientation;

            if (_previewProperties != null)
            {
                await SetPreviewRotationAsync();
            }
            await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () => UpdateButtonOrientation());
        }

        /// <summary>
        /// Take photo button
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private async void PhotoButton_Click(object sender, RoutedEventArgs e)
        {
            await TakePhotoAsync();
        }

        /// <summary>
        /// Face detection button. Enable or disable the face detection effect depending on the previous state. 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private async void FaceDetectionButton_Click(object sender, RoutedEventArgs e)
        {
            // If face detection effect is not enabled or its null, enable it 
            if (_faceDetectionEffect == null || !_faceDetectionEffect.Enabled)
            {
                // Clear any rectangles that may have been left over from a previous instance of the effect
                FacesCanvas.Children.Clear();

                // Create face detection effect (draw bounding boxes) 
                await CreateFaceDetectionEffectAsync();
            }

            // If face detection effect is enabled, disable it 
            else
            {
                // Cleanup the detection effect (draw bounding boxes)
                await CleanUpFaceDetectionEffectAsync();
            }
            // Update the controls 
            UpdateCaptureControls();
        }

        /// <summary>
        /// If maximum video file is taken, limit it so that it does not overflow the device memory. 
        /// In practice, this will not happen as video button is disabled. This will be useful if video button is required.
        /// </summary>
        /// <param name="sender"></param>
        private async void MediaCapture_RecordLimitationExceeded(MediaCapture sender)
        {
            // This is a notification that recording has to stop, and the app is expected to finalize the recording
            await StopRecordingAsync();

            await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () => UpdateCaptureControls());
        }

        /// <summary>
        /// Function if it cannot take photos
        /// In practice, this should not be happening. If it happens, check if camera is being detected 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="errorEventArgs"></param>
        private async void MediaCapture_Failed(MediaCapture sender, MediaCaptureFailedEventArgs errorEventArgs)
        {
            Debug.WriteLine("MediaCapture_Failed: (0x{0:X}) {1}", errorEventArgs.Code, errorEventArgs.Message);

            await CleanupCameraAsync();

            await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () => UpdateCaptureControls());
        }
        
        /// <summary>
        /// Adding face detection effect by highlighting the faces with bounding boxes. 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="args"></param>
        private async void FaceDetectionEffect_FaceDetected(FaceDetectionEffect sender, FaceDetectedEventArgs args)
        {
            // Ask the UI thread to render the face bounding boxes
            await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () => HighlightDetectedFaces(args.ResultFrame.DetectedFaces));
        }

        /// <summary>
        /// Initialize the camera asynchronously. 
        /// </summary>
        /// <returns></returns>
        private async Task InitializeCameraAsync()
        {
            Debug.WriteLine("InitializeCameraAsync");

            // If theres no media capture 
            if (_mediaCapture == null)
            {
                // Attempt to get the front camera if one is available, but use any camera device if not
                var cameraDevice = await FindCameraDeviceByPanelAsync(Windows.Devices.Enumeration.Panel.Front);

                // If fail to find any camera 
                if (cameraDevice == null)
                {
                    Debug.WriteLine("No camera device found!");
                    return;
                }

                // Create MediaCapture and its settings
                _mediaCapture = new MediaCapture();

                // Register for a notification when video recording has reached the maximum time and when something goes wrong
                _mediaCapture.RecordLimitationExceeded += MediaCapture_RecordLimitationExceeded;
                _mediaCapture.Failed += MediaCapture_Failed;

                var settings = new MediaCaptureInitializationSettings { VideoDeviceId = cameraDevice.Id };

                // Initialize MediaCapture
                // It will catch the error if program is not granted access to the camera 
                try
                {
                    await _mediaCapture.InitializeAsync(settings);
                    _isInitialized = true;
                }
                catch (UnauthorizedAccessException)
                {
                    Debug.WriteLine("The app was denied access to the camera");
                }

                // If initialization succeeded, start the preview
                if (_isInitialized)
                {
                    // Figure out where the camera is located
                    if (cameraDevice.EnclosureLocation == null || cameraDevice.EnclosureLocation.Panel == Windows.Devices.Enumeration.Panel.Unknown)
                    {
                        // No information on the location of the camera, assume it's an external camera, not integrated on the device
                        _externalCamera = true;
                    }
                    else
                    {
                        // Camera is fixed on the device
                        _externalCamera = false;

                        // Only mirror the preview if the camera is on the front panel
                        _mirroringPreview = (cameraDevice.EnclosureLocation.Panel == Windows.Devices.Enumeration.Panel.Front);
                    }

                    await StartPreviewAsync();

                    UpdateCaptureControls();
                }
            }
        }

        /// <summary>
        /// Start the preview of the program. 
        /// </summary>
        /// <returns></returns>
        private async Task StartPreviewAsync()
        {
            // Prevent the device from sleeping while the preview is running
            _displayRequest.RequestActive();

            // Set the preview source in the UI and mirror it if necessary
            PreviewControl.Source = _mediaCapture;
            PreviewControl.FlowDirection = _mirroringPreview ? FlowDirection.RightToLeft : FlowDirection.LeftToRight;
    
            // Start the preview
            await _mediaCapture.StartPreviewAsync();
            _previewProperties = _mediaCapture.VideoDeviceController.GetMediaStreamProperties(MediaStreamType.VideoPreview);

            // Initialize the preview to the current orientation
            if (_previewProperties != null)
            {
                _displayOrientation = _displayInformation.CurrentOrientation;

                await SetPreviewRotationAsync();
            }
        }

        private async Task SetPreviewRotationAsync()
        {
            // Only need to update the orientation if the camera is mounted on the device
            if (_externalCamera) return;

            // Calculate which way and how far to rotate the preview
            int rotationDegrees = ConvertDisplayOrientationToDegrees(_displayOrientation);

            // The rotation direction needs to be inverted if the preview is being mirrored
            if (_mirroringPreview)
            {
                rotationDegrees = (360 - rotationDegrees) % 360;
            }

            // Add rotation metadata to the preview stream to make sure the aspect ratio / dimensions match when rendering and getting preview frames
            var props = _mediaCapture.VideoDeviceController.GetMediaStreamProperties(MediaStreamType.VideoPreview);
            props.Properties.Add(RotationKey, rotationDegrees);
            await _mediaCapture.SetEncodingPropertiesAsync(MediaStreamType.VideoPreview, props, null);
        }

        /// <summary>
        /// Stops the preview and deactivates a display request, to allow the screen to go into power saving modes
        /// </summary>
        /// <returns></returns>
        private async Task StopPreviewAsync()
        {
            // Stop the preview
            _previewProperties = null;
            await _mediaCapture.StopPreviewAsync();

            // Use the dispatcher because this method is sometimes called from non-UI threads
            await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {
                // Cleanup the UI
                PreviewControl.Source = null;

                // Allow the device screen to sleep now that the preview is stopped
                _displayRequest.RequestRelease();
            });
        }

        /// <summary>
        /// Create face detection effect asynchronously. 
        /// </summary>
        /// <returns></returns>
        private async Task CreateFaceDetectionEffectAsync()
        {
            // Create the definition, which will contain some initialization settings
            var definition = new FaceDetectionEffectDefinition();

            // To ensure preview smoothness, do not delay incoming samples
            definition.SynchronousDetectionEnabled = false;

            // In this scenario, choose detection speed over accuracy
            definition.DetectionMode = FaceDetectionMode.HighPerformance;

            // Add the effect to the preview stream
            _faceDetectionEffect = (FaceDetectionEffect)await _mediaCapture.AddVideoEffectAsync(definition, MediaStreamType.VideoPreview);

            // Register for face detection events
            _faceDetectionEffect.FaceDetected += FaceDetectionEffect_FaceDetected;

            // Choose the shortest interval between detection events
            _faceDetectionEffect.DesiredDetectionInterval = TimeSpan.FromMilliseconds(20);

            // Start detecting faces
            _faceDetectionEffect.Enabled = true;
        }

        /// <summary>
        ///  Disables and removes the face detection effect, and unregisters the event handler for face detection
        /// </summary>
        /// <returns></returns>
        private async Task CleanUpFaceDetectionEffectAsync()
        {
            // Disable detection
            _faceDetectionEffect.Enabled = false;

            // Unregister the event handler
            _faceDetectionEffect.FaceDetected -= FaceDetectionEffect_FaceDetected;

            // Remove the effect (see ClearEffectsAsync method to remove all effects from a stream)
            await _mediaCapture.RemoveEffectAsync(_faceDetectionEffect);

            // Clear the member variable that held the effect instance
            _faceDetectionEffect = null;
        }

        /// <summary>
        /// Takes a photo to a StorageFile and adds rotation metadata to it
        /// </summary>
        /// <returns></returns>
        private async Task TakePhotoAsync()
        {
            // While taking a photo, keep the video button enabled only if the camera supports simultaneously taking pictures and recording video
            //VideoButton.IsEnabled = _mediaCapture.MediaCaptureSettings.ConcurrentRecordAndPhotoSupported;

            // Make the button invisible if it's disabled, so it's obvious it cannot be interacted with
            //VideoButton.Opacity = VideoButton.IsEnabled ? 1 : 0;

            var stream = new InMemoryRandomAccessStream();

            // Took photo
            Debug.WriteLine("Taking photo...");
            // Capture photo into a stream
            await _mediaCapture.CapturePhotoToStreamAsync(ImageEncodingProperties.CreateJpeg(), stream);

            // Try saving it into the file
            try
            {
                // Save file into picture library as the .jpg name, replace it everytime new picture is taken
                var file = await _captureFolder.CreateFileAsync("SimplePhoto.jpg", CreationCollisionOption.ReplaceExisting);
                Debug.WriteLine("Photo taken! Saving to " + file.Path);

                // Save photo according to the camera orientation (landscape on most of the cases)
                var photoOrientation = ConvertOrientationToPhotoOrientation(GetCameraOrientation());

                // Save it into file 
                await ReencodeAndSavePhotoAsync(stream, file, photoOrientation);

                Debug.WriteLine("Photo saved!");

                // Call the sendFile function to send picture to the server
                if (socketconnect == 1)
                {
                    SendFile();
                }
            }
            catch (Exception ex)
            {
                // File I/O errors are reported as exceptions
                Debug.WriteLine("Exception when taking a photo: " + ex.ToString());
            }
            // Done taking a photo, so re-enable the button
            //VideoButton.IsEnabled = true;
            //VideoButton.Opacity = 1;
        }
        #endregion

        /// <summary>
        /// Senior design: Added connection to the server to perform face recognition.
        /// </summary>
        #region Senior Design Fall 2023 / Spring 2024


        // Variables for server 
        static TcpClient client;
        static NetworkStream stream;
        private int port;
        private string ip;
        int socketconnect = 0;


        /// <summary>
        /// Connect button. If pressed, the program will try to connect to the server via the port and ip provided.
        /// Use your own port and ip. The following port and ip is specific to the port and ip of the Amazon EC2 server program port and ip.  
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private async void Button_Click(object sender, RoutedEventArgs e)
        {
            // Connect to the server 
            if (socketconnect == 0)
            {
                // IP and port for Amazon EC2 server
                // Please use your own IP and port 
                port = 0;
                ip = "";

                // Try to connect 
                try
                {
                    button_server.Opacity = 0;
                    button_server.Visibility = Visibility.Collapsed;
                    // Create a new TcpClient and try to connect 
                    client = new TcpClient();
                    client.Connect(ip, port);
                    stream = client.GetStream();
              
                    // If its connected
                    if (client.Connected)
                    {
                        // Wait to receive "AllConnectionsMade" from the server
                        // Server will send this signal once the connections from SmartMesh IP and this program are established 
                        byte[] receiveddata = new Byte[1024];
                        int bytesRead;
                        string receivedmessage;
                        do
                        {
                            if (stream.DataAvailable)
                            {
                                bytesRead = await stream.ReadAsync(receiveddata, 0, receiveddata.Length);
                                receivedmessage = System.Text.Encoding.ASCII.GetString(receiveddata, 0, bytesRead);
                                Debug.WriteLine(receivedmessage);
                                if (String.Compare(receivedmessage, "AllConnectionsMade\n\0", StringComparison.Ordinal) == 0)
                                {
                                    break;
                                }
                            }
                        } while (true);
                        Debug.WriteLine("Connected Successfully");
                    }

                    // Allow the program to send pictures to the server 
                    socketconnect = 1;
                    // Makes the connect button disappear and disconnect button appear  
                    button_server.Opacity = 0;
                    button_server.Visibility = Visibility.Collapsed;
                    button_disconnect.Opacity = 1;
                    button_disconnect.Visibility = Visibility.Visible;
                }
                // Server did not connect, try again  
                catch
                {
                    Debug.WriteLine("Not connected");
                    button_server.Opacity = 1;
                    button_server.Visibility = Visibility.Visible;
                }
            }   
        }

        /// <summary>
        /// Disconnect button. If preseed, the headset will disconnect from the Amazon EC2 server.  
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private async void button_disconnect_click(object sender, RoutedEventArgs e)
        {
            // Disconnect from the server
            try
            {
                // Write disconnect to the server to acknowledge the disconnection between server and program. 
                byte[] receiveddata = new Byte[1024];
                int bytesRead;
                string receivedmessage;
                byte[] disconnect = Encoding.ASCII.GetBytes("Disconnect");
                await stream.WriteAsync(disconnect, 0, disconnect.Length);

                // Wait to receive "OK" from the server
                do
                {
                    if (stream.DataAvailable)
                    {
                        bytesRead = await stream.ReadAsync(receiveddata, 0, receiveddata.Length);
                        receivedmessage = System.Text.Encoding.ASCII.GetString(receiveddata, 0, bytesRead);
                        Debug.WriteLine(receivedmessage);
                        if (String.Compare(receivedmessage, "OK\n\0", StringComparison.Ordinal) == 0)
                        {
                            break;
                        }
                    }
                } while (true);

                // Close resources in the reverse order of creation
                // Close the stream
                if (stream != null)
                {
                    stream.Flush();
                    stream.Dispose();
                    stream.Close();
                    Debug.WriteLine("Stream Closed");
                }
                // Close the client 
                if (client != null)
                {
                    client.Dispose();
                    client.Close();
                    Debug.WriteLine("Client Closed");
                }

                // Makes the disconnect button disappear and sconnect button appear  
                button_server.Opacity = 1;
                button_server.Visibility = Visibility.Visible;
                button_disconnect.Opacity = 1;
                button_disconnect.Visibility = Visibility.Collapsed;
                // Do not allow the program to send pictures to the server
                socketconnect = 0;
                Debug.WriteLine("Disconnected Successfully");
            }
            catch
            {
                Debug.WriteLine("Not disconnected");
            }
        }

        // Variables for sending data
        byte[] textimage = Encoding.ASCII.GetBytes("Image");
        byte[] writetextdata = Encoding.ASCII.GetBytes("SensorData");
        byte[] nofaces = Encoding.ASCII.GetBytes("NoFaces");
        byte[] OK = Encoding.ASCII.GetBytes("OK");

        // Make the receivedNames into a list of string
        List<string> receivedNames = new List<string>();

        /// <summary>
        /// Send file function once picture is taken. This will happen IF and ONLY IF the program is connected to the server. 
        /// If connected, the picture will be cropped and send to the server in black and white format IF faces are detected. 
        /// If there's no faces detected, the entire picture file will be sent to the server. 
        /// If its not connected, the SendFile function will not send any cropped pictures / entire picture. 
        /// Reference: 
        ///    
        ///https://learn.microsoft.com/en-us/windows/uwp/files/quickstart-reading-and-writing-files
        ///https://learn.microsoft.com/en-us/answers/questions/1320990/how-to-send-receive-enough-data-byte-in-networkstr
        ///https://stackoverflow.com/questions/13026666/sending-a-large-amount-of-data-throught-tcp-socket
        ///
        /// </summary>
        public async void SendFile()
        {
            // Do not display name until all names are received
            allowdisplayname = 0;
            // Temporarily disable the photo button
            PhotoButton.Visibility = Visibility.Collapsed;
            PhotoButton.Opacity = 0;

            // Get the picture from the folder, and find the specific name of the image saved 
            Windows.Storage.StorageFile sampleFile = await _captureFolder.GetFileAsync("SimplePhoto.jpg");

            // Using software bitmap to convert picture into greyscale, for facial recognition purpose on the server 
            SoftwareBitmap softwareBitmap;

            // Open the picture taken for read into a stream  
            using (var stream = await sampleFile.OpenStreamForReadAsync())
            {
                // Conver the picture taken into a grayscale 8-bit format 
                Windows.Storage.Streams.IRandomAccessStream randomAccessStream = stream.AsRandomAccessStream();
                var decoder = await BitmapDecoder.CreateAsync(randomAccessStream);
                var softwareBitmapTemp = await decoder.GetSoftwareBitmapAsync();
                softwareBitmap = SoftwareBitmap.Convert(softwareBitmapTemp, BitmapPixelFormat.Gray8);
            }
            // Wait for face detector to be created and detect number of faces 
            FaceDetector faceDetector = await FaceDetector.CreateAsync();
            var faces = await faceDetector.DetectFacesAsync(softwareBitmap);

            // Clear all the names inside the list 
            receivedNames.Clear();

            // If face is detected
            if (faces.Count > 0)
            {
                // Sort faces by order, from left to right (If not mirrored)
                // The faces will be sorted from right to left on a normal PC screen with camera facing IN your face. 
                // The faces will be sorted from left to right on the headset with camera facing away from your face. 
                var sortedFaces = faces.OrderBy(f => f.FaceBox.X).ToList();

                // Crop and save each detected face
                for (int i = 0; i < sortedFaces.Count; i++)
                {
                    // Take the number of faces and sort it 
                    var face = sortedFaces[i];

                    // Create a BitmapBounds to represent the face rectangle
                    BitmapBounds faceBounds = new BitmapBounds
                    {
                        X = (uint)face.FaceBox.X,
                        Y = (uint)face.FaceBox.Y,
                        Width = (uint)face.FaceBox.Width,
                        Height = (uint)face.FaceBox.Height
                    };

                    // Replace the old file with the current file, to save memory / space. 
                    StorageFolder picturesFolder = KnownFolders.PicturesLibrary;
                    StorageFile croppedFile = await picturesFolder.CreateFileAsync($"CroppedFace{i}.jpg", CreationCollisionOption.ReplaceExisting);
                    
                    // Save the cropped faces into the folder 
                    using (var croppedStream = await croppedFile.OpenAsync(FileAccessMode.ReadWrite))
                    {
                        var encoder = await BitmapEncoder.CreateAsync(BitmapEncoder.JpegEncoderId, croppedStream);
                        encoder.BitmapTransform.Bounds = faceBounds;
                        var convertedBitmap = SoftwareBitmap.Convert(softwareBitmap, BitmapPixelFormat.Bgra8);
                        encoder.SetSoftwareBitmap(convertedBitmap);
                        await encoder.FlushAsync();
                    }
                }

                // Send the number of images the server will receive
                int NumberOfFaces = faces.Count;
                string NumberAsString = NumberOfFaces.ToString();
                byte[] writenumberofimage = Encoding.ASCII.GetBytes(NumberAsString);
                await stream.WriteAsync(writenumberofimage, 0, writenumberofimage.Length);

                //Stopwatch timer = new Stopwatch();
                //timer.Start();

                // Wait for the word "OK" from the server as acknowledgement that it receives number of faces. 
                do
                {
                    byte[] receiveddata = new Byte[1024];
                    int bytesRead;
                    string receivedmessage;

                    if (stream.DataAvailable)
                    {
                        bytesRead = await stream.ReadAsync(receiveddata, 0, receiveddata.Length);
                        receivedmessage = System.Text.Encoding.ASCII.GetString(receiveddata, 0, bytesRead);
                        Debug.WriteLine(receivedmessage);
                        if (String.Compare(receivedmessage, "OK\n\0", StringComparison.Ordinal) == 0)
                        {
                            break;
                        }
                    }
                } while (true);
                
                
                // Send the number of pictures according to the number of faces detected on the picture taken 
                for (int i = 0; i < faces.Count; i++)
                {
                    // Result to convert
                    byte[] result;
                    // Get the cropped image 
                    Windows.Storage.StorageFile croppedimage = await _captureFolder.GetFileAsync($"CroppedFace{i}.jpg");

                    //Convert the photo to a byte of array using stream 
                    using (Stream stream = await croppedimage.OpenStreamForReadAsync())
                    {
                        using (var memoryStream = new MemoryStream())
                        {
                            stream.CopyTo(memoryStream);
                            result = memoryStream.ToArray();
                        }
                    }

                    // Now try to send the data via socket
                    try
                    {
                        // Split the chunk size so that data is sent 1024 per packet 
                        int chunkSize = 1024;
                        // Offset for reading bytes
                        int offset = 0;
                        // Convert the length of array into integer
                        int resultlength = result.Length;
                        // Convert the result into bytes
                        byte[] resultlengthbytes = BitConverter.GetBytes(resultlength);

                        // Check if socket is connected
                        if (socketconnect == 1)
                        {
                            // Variables to read data from the server
                            byte[] receiveddata = new Byte[1024];
                            int bytesRead;
                            string receivedmessage;
                            string cachename;

                            // Write the word "Image" to the server
                            await stream.WriteAsync(textimage, 0, textimage.Length);

                            // Wait for the word "OK" from the server as acknowledgement that it receives "Image". 
                            do
                            {
                                if (stream.DataAvailable)
                                {
                                    bytesRead = await stream.ReadAsync(receiveddata, 0, receiveddata.Length);
                                    receivedmessage = System.Text.Encoding.ASCII.GetString(receiveddata, 0, bytesRead);
                                    Debug.WriteLine(receivedmessage);
                                    if (String.Compare(receivedmessage, "OK\n\0", StringComparison.Ordinal) == 0)
                                    {
                                        break;
                                    }
                                }
                            } while (true);

                            // Write the length of data of the captured image
                            await stream.WriteAsync(resultlengthbytes, 0, resultlengthbytes.Length);

                            // Wait for the word "OK" from the server as acknowledgement that it receives the length of data. 
                            do
                            {
                                if (stream.DataAvailable)
                                {
                                    bytesRead = await stream.ReadAsync(receiveddata, 0, receiveddata.Length);
                                    receivedmessage = System.Text.Encoding.ASCII.GetString(receiveddata, 0, bytesRead);
                                    Debug.WriteLine(receivedmessage);
                                    if (String.Compare(receivedmessage, "OK\n\0", StringComparison.Ordinal) == 0)
                                    {
                                        break;
                                    }
                                }
                            } while (true);

                            // Check for remaining data to be sent
                            int remaining;
                            
                            // Data chunk size to be sent
                            int chunk;

                            // Loop it until all data is sent
                            while (offset < result.Length)
                            {
                                // The remaining data to be sent
                                remaining = result.Length - offset;
                                
                                // Compare if chunkSize or remaining is smaller and take that value, used for sending last packet of data
                                chunk = Math.Min(chunkSize, remaining);
                                
                                // Send the result of 1024 bytes to the server
                                await stream.WriteAsync(result, offset, chunk);

                                // Add offset to send new packet 
                                offset += chunk;
                                // Wait for the word "OK" from the server as acknowledgement that it receives the packet. 
                                do
                                {
                                    if (stream.DataAvailable)
                                    {
                                        bytesRead = await stream.ReadAsync(receiveddata, 0, receiveddata.Length);
                                        receivedmessage = System.Text.Encoding.ASCII.GetString(receiveddata, 0, bytesRead);
                                        Debug.WriteLine(receivedmessage);
                                        if (String.Compare(receivedmessage, "OK\n\0", StringComparison.Ordinal) == 0)
                                        {
                                            break;
                                        }
                                    }
                                } while (true);

                            }

                            // Final packet is sent, server should receive all data it needs now. 
                            // Wait for the word "Done" from the server as acknowledgement that it receives all the packets. 
                            do
                            {
                                if (stream.DataAvailable)
                                {
                                    bytesRead = await stream.ReadAsync(receiveddata, 0, receiveddata.Length);
                                    receivedmessage = System.Text.Encoding.ASCII.GetString(receiveddata, 0, bytesRead);
                                    Debug.WriteLine(receivedmessage);
                                    if (String.Compare(receivedmessage, "Done\n\0", StringComparison.Ordinal) == 0)
                                    {
                                        break;
                                    }
                                }
                            } while (true);

                            // Write the word "SensorData" to the server
                            await stream.WriteAsync(writetextdata, 0, writetextdata.Length);

                            // Wait for the word "OK" from the server as acknowledgement that it receives "SensorData".
                            do
                            {
                                if (stream.DataAvailable)
                                {
                                    bytesRead = await stream.ReadAsync(receiveddata, 0, receiveddata.Length);
                                    receivedmessage = System.Text.Encoding.ASCII.GetString(receiveddata, 0, bytesRead);
                                    Debug.WriteLine(receivedmessage);
                                    if (String.Compare(receivedmessage, "OK\n\0", StringComparison.Ordinal) == 0)
                                    {
                                        break;
                                    }
                                }
                            } while (true);

                            // Variables to convert number into a string 
                            string pointlatitudestring = Convert.ToString(pointlatitude);
                            string pointlongtitudestring = Convert.ToString(pointlongtitude);
                            string altitudestring = Convert.ToString(fromsealevel);

                            // Add the string together to send, seperated by a comma 
                            string stringaddtogether = pointlatitudestring + "," + pointlongtitudestring + "," + altitudestring;
                            byte[] longlattext = Encoding.ASCII.GetBytes(stringaddtogether);

                            // Write the sensor data value to the server 
                            await stream.WriteAsync(longlattext, 0, longlattext.Length);

                            // Wait for the word "OK" from the server as acknowledgement that it receives sensor data value. 
                            do
                            {
                                if (stream.DataAvailable)
                                {
                                    bytesRead = await stream.ReadAsync(receiveddata, 0, receiveddata.Length);
                                    receivedmessage = System.Text.Encoding.ASCII.GetString(receiveddata, 0, bytesRead);
                                    Debug.WriteLine(receivedmessage);
                                    if (String.Compare(receivedmessage, "OK\n\0", StringComparison.Ordinal) == 0)
                                    {
                                        break;
                                    }
                                }
                            } while (true);

                            // Server now sends back the name 
                            bytesRead = await stream.ReadAsync(receiveddata, 0, receiveddata.Length);
                            receivedmessage = System.Text.Encoding.ASCII.GetString(receiveddata, 0, bytesRead);
                            Debug.WriteLine(receivedmessage);
                            // Display name for debug purpose 
                            TextBox9.Text = "";
                            TextBox9.Text = receivedmessage;
                            // Use a cahce variable to store the name (not needed anymore)
                            cachename = receivedmessage;

                            // Write the word "OK" to the server 
                            await stream.WriteAsync(OK,0,OK.Length);
                            /*
                            Wait for the word "Complete" from the server as acknowledgement that all data
                            (pictures and sensor data) are transmitted.
                            */
                            do
                            {
                                if (stream.DataAvailable)
                                {
                                    bytesRead = await stream.ReadAsync(receiveddata, 0, receiveddata.Length);
                                    receivedmessage = System.Text.Encoding.ASCII.GetString(receiveddata, 0, bytesRead);
                                    Debug.WriteLine(receivedmessage);
                                    if (String.Compare(receivedmessage, "Complete\n\0", StringComparison.Ordinal) == 0)
                                    {
                                        break;
                                    }
                                }
                            } while (true);
                            
                            //Call the function to add names into a list, for display purposes on the face box later. 
                            HandleReceivedName(cachename);
                        }
                    }
                    catch
                    {
                        Debug.WriteLine("Error, could not write data to the server!");
                    }
                }
            }

            // If faces are not detected, send the word "NoFaces" to the server so that the server does not have to perform face recognition. 
            // Saves bandwitdh on all sites of connection (Headset / PC, server, and SmartMeshIP).
            else
            {
                await stream.WriteAsync(nofaces, 0, nofaces.Length);
                Debug.WriteLine("NoFaces");
                do
                {
                    byte[] receiveddata = new Byte[1024];
                    int bytesRead;
                    string receivedmessage;

                    if (stream.DataAvailable)
                    {
                        bytesRead = await stream.ReadAsync(receiveddata, 0, receiveddata.Length);
                        receivedmessage = System.Text.Encoding.ASCII.GetString(receiveddata, 0, bytesRead);
                        Debug.WriteLine(receivedmessage);
                        if (String.Compare(receivedmessage, "Complete\n\0", StringComparison.Ordinal) == 0)
                        {
                            break;
                        }
                    }
                } while (true);
            }
            // Reenable the photo button 
            PhotoButton.Visibility = Visibility.Visible;
            PhotoButton.Opacity = 1;
            // Allow names to be displayed on the face box 
            allowdisplayname = 1;
        }

        /// <summary>
        /// Add the names received into a list. Will be used for display later. 
        /// </summary>
        /// <param name="name"></param>
        private void HandleReceivedName(string name)
        {
            receivedNames.Add(name);
        }
        #endregion


        #region Vidoe features (Not being used)
        /*
        The function (VideoButton_Click) and two tasks below are not in use (StartRecordingAsync and StopRecordingAsync) 
        as VideoButton is disabled.
        
        Will be helpful if Video is required.
        */

        /// <summary>
        /// Take video button, not in use anymore.
        /// Reenable it by allowing the video button to reappear under MainPage.xaml with the appropiate effect when its clicked. 
        /// Follow Microsoft's UWP Guideline for more information.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private async void VideoButton_Click(object sender, RoutedEventArgs e)
        {
            /*
            if (!_isRecording)
            {
                await StartRecordingAsync();
            }
            else
            {
                await StopRecordingAsync();
            }

            // After starting or stopping video recording, update the UI to reflect the MediaCapture state
            UpdateCaptureControls();
            */
        }


        /// <summary>
        /// Records an MP4 video to a StorageFile and adds rotation metadata to it
        /// </summary>
        /// <returns></returns>
        private async Task StartRecordingAsync()
        {
            try
            {
                // Create storage file for the capture
                var videoFile = await _captureFolder.CreateFileAsync("SimpleVideo.mp4", CreationCollisionOption.GenerateUniqueName);

                var encodingProfile = MediaEncodingProfile.CreateMp4(VideoEncodingQuality.Auto);

                // Calculate rotation angle, taking mirroring into account if necessary
                var rotationAngle = 360 - ConvertDeviceOrientationToDegrees(GetCameraOrientation());
                encodingProfile.Video.Properties.Add(RotationKey, PropertyValue.CreateInt32(rotationAngle));

                Debug.WriteLine("Starting recording to " + videoFile.Path);

                await _mediaCapture.StartRecordToStorageFileAsync(encodingProfile, videoFile);
                _isRecording = true;

                Debug.WriteLine("Started recording!");
            }
            catch (Exception ex)
            {
                // File I/O errors are reported as exceptions
                Debug.WriteLine("Exception when starting video recording: " + ex.ToString());
            }
        }

        /// <summary>
        /// Stops recording a video
        /// </summary>
        /// <returns></returns>
        private async Task StopRecordingAsync()
        {
            Debug.WriteLine("Stopping recording...");

            _isRecording = false;
            await _mediaCapture.StopRecordAsync();

            Debug.WriteLine("Stopped recording!");
        }

        /// <summary>
        /// Cleans up the camera resources (after stopping any video recording and/or preview if necessary) and unregisters from MediaCapture events
        /// </summary>
        /// <returns></returns>
        private async Task CleanupCameraAsync()
        {
            Debug.WriteLine("CleanupCameraAsync");

            if (_isInitialized)
            {
                // If a recording is in progress during cleanup, stop it to save the recording
                if (_isRecording)
                {
                    await StopRecordingAsync();
                }

                if (_faceDetectionEffect != null)
                {
                    await CleanUpFaceDetectionEffectAsync();
                }

                if (_previewProperties != null)
                {
                    // The call to stop the preview is included here for completeness, but can be
                    // safely removed if a call to MediaCapture.Dispose() is being made later,
                    // as the preview will be automatically stopped at that point
                    await StopPreviewAsync();
                }

                _isInitialized = false;
            }

            if (_mediaCapture != null)
            {
                _mediaCapture.RecordLimitationExceeded -= MediaCapture_RecordLimitationExceeded;
                _mediaCapture.Failed -= MediaCapture_Failed;
                _mediaCapture.Dispose();
                _mediaCapture = null;
            }
        }
        #endregion

        #region Extra setups for UI / cameras
        private async Task SetupUiAsync()
        {
            // Attempt to lock page to landscape orientation to prevent the CaptureElement from rotating, as this gives a better experience
            DisplayInformation.AutoRotationPreferences = DisplayOrientations.Landscape;

            // Populate orientation variables with the current state
            _displayOrientation = _displayInformation.CurrentOrientation;
            if (_orientationSensor != null)
            {
                _deviceOrientation = _orientationSensor.GetCurrentOrientation();
            }

            RegisterEventHandlers();

            var picturesLibrary = await StorageLibrary.GetLibraryAsync(KnownLibraryId.Pictures);
            // Fall back to the local app storage if the Pictures Library is not available
            _captureFolder = picturesLibrary.SaveFolder ?? ApplicationData.Current.LocalFolder;
        }

        /// <summary>
        /// Unregisters event handlers for hardware buttons and orientation sensors, allows the StatusBar (on Phone) to show, and removes the page orientation lock
        /// </summary>
        /// <returns></returns>
        private async Task CleanupUiAsync()
        {
            UnregisterEventHandlers();

            // Revert orientation preferences
            DisplayInformation.AutoRotationPreferences = DisplayOrientations.None;
        }

        /// <summary>
        /// This method will update the icons, enable/disable and show/hide the photo/video buttons depending on the current state of the app and the capabilities of the device
        /// </summary>
        private void UpdateCaptureControls()
        {
            // The buttons should only be enabled if the preview started sucessfully
            PhotoButton.IsEnabled = _previewProperties != null;
            VideoButton.IsEnabled = _previewProperties != null;
            FaceDetectionButton.IsEnabled = _previewProperties != null;

            // Update the face detection icon depending on whether the effect exists or not
            FaceDetectionDisabledIcon.Visibility = (_faceDetectionEffect == null || !_faceDetectionEffect.Enabled) ? Visibility.Visible : Visibility.Collapsed;
            FaceDetectionEnabledIcon.Visibility = (_faceDetectionEffect != null && _faceDetectionEffect.Enabled) ? Visibility.Visible : Visibility.Collapsed;

            // Hide the face detection canvas and clear it
            FacesCanvas.Visibility = (_faceDetectionEffect != null && _faceDetectionEffect.Enabled) ? Visibility.Visible : Visibility.Collapsed;

            // Update recording button to show "Stop" icon instead of red "Record" icon when recording
            StartRecordingIcon.Visibility = _isRecording ? Visibility.Collapsed : Visibility.Visible;
            StopRecordingIcon.Visibility = _isRecording ? Visibility.Visible : Visibility.Collapsed;

            // If the camera doesn't support simultaneosly taking pictures and recording video, disable the photo button on record
            if (_isInitialized && !_mediaCapture.MediaCaptureSettings.ConcurrentRecordAndPhotoSupported)
            {
                PhotoButton.IsEnabled = !_isRecording;

                // Make the button invisible if it's disabled, so it's obvious it cannot be interacted with
                PhotoButton.Opacity = PhotoButton.IsEnabled ? 1 : 0;
            }
        }

        /// <summary>
        /// Registers event handlers for hardware buttons and orientation sensors, and performs an initial update of the UI rotation
        /// </summary>
        private void RegisterEventHandlers()
        {
            // If there is an orientation sensor present on the device, register for notifications
            if (_orientationSensor != null)
            {
                _orientationSensor.OrientationChanged += OrientationSensor_OrientationChanged;

                // Update orientation of buttons with the current orientation
                UpdateButtonOrientation();
            }

            _displayInformation.OrientationChanged += DisplayInformation_OrientationChanged;
            _systemMediaControls.PropertyChanged += SystemMediaControls_PropertyChanged;
        }

        /// <summary>
        /// Unregisters event handlers for hardware buttons and orientation sensors
        /// </summary>
        private void UnregisterEventHandlers()
        {

            if (_orientationSensor != null)
            {
                _orientationSensor.OrientationChanged -= OrientationSensor_OrientationChanged;
            }

            _displayInformation.OrientationChanged -= DisplayInformation_OrientationChanged;
            _systemMediaControls.PropertyChanged -= SystemMediaControls_PropertyChanged;
        }

        /// <summary>
        /// Attempts to find and return a device mounted on the panel specified, and on failure to find one it will return the first device listed
        /// </summary>
        /// <param name="desiredPanel">The desired panel on which the returned device should be mounted, if available</param>
        /// <returns></returns>
        private static async Task<DeviceInformation> FindCameraDeviceByPanelAsync(Windows.Devices.Enumeration.Panel desiredPanel)
        {
            // Get available devices for capturing pictures
            var allVideoDevices = await DeviceInformation.FindAllAsync(DeviceClass.VideoCapture);

            // Get the desired camera by panel
            DeviceInformation desiredDevice = allVideoDevices.FirstOrDefault(x => x.EnclosureLocation != null && x.EnclosureLocation.Panel == desiredPanel);

            // If there is no device mounted on the desired panel, return the first device found
            return desiredDevice ?? allVideoDevices.FirstOrDefault();
        }

        /// <summary>
        /// Applies the given orientation to a photo stream and saves it as a StorageFile
        /// </summary>
        /// <param name="stream">The photo stream</param>
        /// <param name="file">The StorageFile in which the photo stream will be saved</param>
        /// <param name="photoOrientation">The orientation metadata to apply to the photo</param>
        /// <returns></returns>
        private static async Task ReencodeAndSavePhotoAsync(IRandomAccessStream stream, StorageFile file, PhotoOrientation photoOrientation)
        {
            using (var inputStream = stream)
            {
                var decoder = await BitmapDecoder.CreateAsync(inputStream);

                using (var outputStream = await file.OpenAsync(FileAccessMode.ReadWrite))
                {
                    var encoder = await BitmapEncoder.CreateForTranscodingAsync(outputStream, decoder);

                    var properties = new BitmapPropertySet { { "System.Photo.Orientation", new BitmapTypedValue(photoOrientation, PropertyType.UInt16) } };

                    await encoder.BitmapProperties.SetPropertiesAsync(properties);
                    await encoder.FlushAsync();
                }
            }
        }

        /// <summary>
        /// Gets the camera orientation. Check if its in landscape or portrait mode. 
        /// If running on PC / headset, it should be running in landscape mode (unless orientation changed by the user to be a non-normal orientation). 
        /// </summary>
        /// <returns></returns>
        private SimpleOrientation GetCameraOrientation()
        {
            if (_externalCamera)
            {
                // Cameras that are not attached to the device do not rotate along with it, so apply no rotation
                return SimpleOrientation.NotRotated;
            }

            var result = _deviceOrientation;

            // Account for the fact that, on portrait-first devices, the camera sensor is mounted at a 90 degree offset to the native orientation
            if (_displayInformation.NativeOrientation == DisplayOrientations.Portrait)
            {
                switch (result)
                {
                    case SimpleOrientation.Rotated90DegreesCounterclockwise:
                        result = SimpleOrientation.NotRotated;
                        break;
                    case SimpleOrientation.Rotated180DegreesCounterclockwise:
                        result = SimpleOrientation.Rotated90DegreesCounterclockwise;
                        break;
                    case SimpleOrientation.Rotated270DegreesCounterclockwise:
                        result = SimpleOrientation.Rotated180DegreesCounterclockwise;
                        break;
                    case SimpleOrientation.NotRotated:
                        result = SimpleOrientation.Rotated270DegreesCounterclockwise;
                        break;
                }
            }

            // If the preview is being mirrored for a front-facing camera, then the rotation should be inverted
            if (_mirroringPreview)
            {
                // This only affects the 90 and 270 degree cases, because rotating 0 and 180 degrees is the same clockwise and counter-clockwise
                switch (result)
                {
                    case SimpleOrientation.Rotated90DegreesCounterclockwise:
                        return SimpleOrientation.Rotated270DegreesCounterclockwise;
                    case SimpleOrientation.Rotated270DegreesCounterclockwise:
                        return SimpleOrientation.Rotated90DegreesCounterclockwise;
                }
            }
            return result;
        }

        /// <summary>
        /// Converts the given orientation of the device in space to the corresponding rotation in degrees
        /// </summary>
        /// <param name="orientation">The orientation of the device in space</param>
        /// <returns>An orientation in degrees</returns>
        private static int ConvertDeviceOrientationToDegrees(SimpleOrientation orientation)
        {
            switch (orientation)
            {
                case SimpleOrientation.Rotated90DegreesCounterclockwise:
                    return 90;
                case SimpleOrientation.Rotated180DegreesCounterclockwise:
                    return 180;
                case SimpleOrientation.Rotated270DegreesCounterclockwise:
                    return 270;
                case SimpleOrientation.NotRotated:
                default:
                    return 0;
            }
        }

        /// <summary>
        /// Converts the given orientation of the app on the screen to the corresponding rotation in degrees
        /// </summary>
        /// <param name="orientation">The orientation of the app on the screen</param>
        /// <returns>An orientation in degrees</returns>
        private static int ConvertDisplayOrientationToDegrees(DisplayOrientations orientation)
        {
            switch (orientation)
            {
                case DisplayOrientations.Portrait:
                    return 90;
                case DisplayOrientations.LandscapeFlipped:
                    return 180;
                case DisplayOrientations.PortraitFlipped:
                    return 270;
                case DisplayOrientations.Landscape:
                default:
                    return 0;
            }
        }

        /// <summary>
        /// Converts the given orientation of the device in space to the metadata that can be added to captured photos
        /// </summary>
        /// <param name="orientation">The orientation of the device in space</param>
        /// <returns></returns>
        private static PhotoOrientation ConvertOrientationToPhotoOrientation(SimpleOrientation orientation)
        {
            switch (orientation)
            {
                case SimpleOrientation.Rotated90DegreesCounterclockwise:
                    return PhotoOrientation.Rotate90;
                case SimpleOrientation.Rotated180DegreesCounterclockwise:
                    return PhotoOrientation.Rotate180;
                case SimpleOrientation.Rotated270DegreesCounterclockwise:
                    return PhotoOrientation.Rotate270;
                case SimpleOrientation.NotRotated:
                default:
                    return PhotoOrientation.Normal;
            }
        }

        /// <summary>
        /// Uses the current device orientation in space and page orientation on the screen to calculate the rotation
        /// transformation to apply to the controls
        /// </summary>
        /// <returns>An angle in degrees to rotate the controls so they remain upright to the user regardless of device and page
        /// orientation</returns>
        private void UpdateButtonOrientation()
        {
            int device = ConvertDeviceOrientationToDegrees(_deviceOrientation);
            int display = ConvertDisplayOrientationToDegrees(_displayOrientation);

            if (_displayInformation.NativeOrientation == DisplayOrientations.Portrait)
            {
                device -= 90;
            }

            // Combine both rotations and make sure that 0 <= result < 360
            var angle = (360 + display + device) % 360;

            // Rotate the buttons in the UI to match the rotation of the device
            var transform = new RotateTransform { Angle = angle };

            // The RenderTransform is safe to use (i.e. it won't cause layout issues) in this case, because these buttons have a 1:1 aspect ratio
            PhotoButton.RenderTransform = transform;
            VideoButton.RenderTransform = transform;
            FaceDetectionButton.RenderTransform = transform;
        }

        /// <summary>
        /// Uses the current display orientation to calculate the rotation transformation to apply to the face detection bounding box canvas
        /// and mirrors it if the preview is being mirrored
        /// </summary>
        private void SetFacesCanvasRotation()
        {
            // Calculate how much to rotate the canvas
            int rotationDegrees = ConvertDisplayOrientationToDegrees(_displayOrientation);

            // The rotation direction needs to be inverted if the preview is being mirrored, just like in SetPreviewRotationAsync
            if (_mirroringPreview)
            {
                rotationDegrees = (360 - rotationDegrees) % 360;
            }

            // Apply the rotation
            var transform = new RotateTransform { Angle = rotationDegrees };
            FacesCanvas.RenderTransform = transform;

            var previewArea = GetPreviewStreamRectInControl(_previewProperties as VideoEncodingProperties, PreviewControl);

            // For portrait mode orientations, swap the width and height of the canvas after the rotation, so the control continues to overlap the preview
            if (_displayOrientation == DisplayOrientations.Portrait || _displayOrientation == DisplayOrientations.PortraitFlipped)
            {
                FacesCanvas.Width = previewArea.Height;
                FacesCanvas.Height = previewArea.Width;

                // The position of the canvas also needs to be adjusted, as the size adjustment affects the centering of the control
                Canvas.SetLeft(FacesCanvas, previewArea.X - (previewArea.Height - previewArea.Width) / 2);
                Canvas.SetTop(FacesCanvas, previewArea.Y - (previewArea.Width - previewArea.Height) / 2);
            }
            else
            {
                FacesCanvas.Width = previewArea.Width;
                FacesCanvas.Height = previewArea.Height;

                Canvas.SetLeft(FacesCanvas, previewArea.X);
                Canvas.SetTop(FacesCanvas, previewArea.Y);
            }

            // Also mirror the canvas if the preview is being mirrored
            FacesCanvas.FlowDirection = _mirroringPreview ? FlowDirection.RightToLeft : FlowDirection.LeftToRight;
        }
        #endregion

        #region Face Detection boxes with names on the boxes
        // Variables to check if the program should clear the names on the boxes. 
        // The names will be cleared under two conditions: 
        // 1. The faces detected are not equal to the number of person present in the frame (more than / less than).
        // 2. The faces detected are 0. 
        int facecache;
        int allowdisplayname;

        /// <summary>
        /// Iterates over all detected faces, creating and adding Rectangles to the FacesCanvas as face bounding boxes
        /// </summary>
        /// <param name="faces">The list of detected faces from the FaceDetected event of the effect</param>
        private async void HighlightDetectedFaces(IReadOnlyList<DetectedFace> faces)
        {
            // Sort faces from left to right, as followed by the sending orientation from left to right to the server. 
            faces = faces.OrderBy(face => face.FaceBox.X).ToList();
            // Remove any existing rectangles from previous events
            FacesCanvas.Children.Clear();

            // If names are allowed to be displayed, check if the conditions to display names are met. 
            // If number of faces = 0 or number of faces not equal to the received names from the server, clear all the names in the list. 
            facecache = faces.Count;
            if (allowdisplayname == 1)
            {
                if ((facecache == 0) || (facecache != receivedNames.Count))
                {
                    receivedNames.Clear();
                }
            }

            // Face boxes color. User can choose to increase or decrease number of colors   
            SolidColorBrush[] faceColors = new SolidColorBrush[]
            {
                new SolidColorBrush(Colors.Blue),
                new SolidColorBrush(Colors.DeepSkyBlue),
                new SolidColorBrush(Colors.Green),
                new SolidColorBrush(Colors.Orange),
                new SolidColorBrush(Colors.Orchid),
                // Add more colors as needed
            };
            
            // For each detected face
            for (int i = 0; i < faces.Count; i++)
            {
                // Face coordinate units are preview resolution pixels, which can be a different scale from our display resolution, so a conversion may be necessary
                Windows.UI.Xaml.Shapes.Rectangle faceBoundingBox = ConvertPreviewToUiRectangle(faces[i].FaceBox);

                // Set bounding box stroke properties
                faceBoundingBox.StrokeThickness = 2;

                // Highlight the first face in the set
                //faceBoundingBox.Stroke = (i == 0 ? new SolidColorBrush(Colors.Blue) : new SolidColorBrush(Colors.DeepSkyBlue));
                faceBoundingBox.Stroke = faceColors[i % faceColors.Length];

                // Add grid to canvas containing all face UI objects
                FacesCanvas.Children.Add(faceBoundingBox);
            }
           

            // Check if there are received names to display
            if (receivedNames != null && receivedNames.Count > 0)
            {
                // Display associated names from left to right
                for (int i = 0; i < Math.Min(faces.Count, receivedNames.Count); i++)
                {
                    // Display the name associated with the face with a larger font size
                    TextBlock nameTextBlock = new TextBlock
                    {
                        Text = receivedNames[i] ?? "", // Use null-coalescing operator to handle null values
                        Foreground = new SolidColorBrush(Colors.Blue), //Use any color as you like, or add colors like faceColors 
                        FontSize = 26, // Set the desired font size (adjust as needed)
                    };

                    // Get the corresponding face bounding box
                    Windows.UI.Xaml.Shapes.Rectangle faceBoundingBox = FacesCanvas.Children[i] as Windows.UI.Xaml.Shapes.Rectangle;

                    // Set the margin to position the name above the face bounding box
                    if (faceBoundingBox != null)
                    {
                        double left = Canvas.GetLeft(faceBoundingBox);
                        double top = Canvas.GetTop(faceBoundingBox) - 30;

                        // If flow direction is flipped (from right to left, running on PC), reverse the name order 
                        if (FacesCanvas.FlowDirection == FlowDirection.RightToLeft)
                        {
                            // Subtract the width of the TextBlock for right-to-left flow
                            left -= nameTextBlock.ActualWidth;
                        }

                        nameTextBlock.Margin = new Thickness(left, top, 0, 0);
                        //nameTextBlock.Margin = new Thickness(Canvas.GetLeft(faceBoundingBox), Canvas.GetTop(faceBoundingBox) - 20, 0, 0);
                        FacesCanvas.Children.Add(nameTextBlock);
                    }
                }
            }

            // Update the face detection bounding box canvas orientation
            SetFacesCanvasRotation();
        }
        
        /// <summary>
        /// Takes face information defined in preview coordinates and returns one in UI coordinates, taking
        /// into account the position and size of the preview control.
        /// </summary>
        /// <param name="faceBoxInPreviewCoordinates">Face coordinates as retried from the FaceBox property of a DetectedFace, in preview coordinates.</param>
        /// <returns>Rectangle in UI (CaptureElement) coordinates, to be used in a Canvas control.</returns>
        private Windows.UI.Xaml.Shapes.Rectangle ConvertPreviewToUiRectangle(BitmapBounds faceBoxInPreviewCoordinates)
        {
            var result = new Windows.UI.Xaml.Shapes.Rectangle();
            var previewStream = _previewProperties as VideoEncodingProperties;

            // If there is no available information about the preview, return an empty rectangle, as re-scaling to the screen coordinates will be impossible
            if (previewStream == null) return result;

            // Similarly, if any of the dimensions is zero (which would only happen in an error case) return an empty rectangle
            if (previewStream.Width == 0 || previewStream.Height == 0) return result;

            double streamWidth = previewStream.Width;
            double streamHeight = previewStream.Height;

            // For portrait orientations, the width and height need to be swapped
            if (_displayOrientation == DisplayOrientations.Portrait || _displayOrientation == DisplayOrientations.PortraitFlipped)
            {
                streamHeight = previewStream.Width;
                streamWidth = previewStream.Height;
            }

            // Get the rectangle that is occupied by the actual video feed
            var previewInUI = GetPreviewStreamRectInControl(previewStream, PreviewControl);

            // Scale the width and height from preview stream coordinates to window coordinates
            result.Width = (faceBoxInPreviewCoordinates.Width / streamWidth) * previewInUI.Width;
            result.Height = (faceBoxInPreviewCoordinates.Height / streamHeight) * previewInUI.Height;

            // Scale the X and Y coordinates from preview stream coordinates to window coordinates
            var x = (faceBoxInPreviewCoordinates.X / streamWidth) * previewInUI.Width;
            var y = (faceBoxInPreviewCoordinates.Y / streamHeight) * previewInUI.Height;
            Canvas.SetLeft(result, x);
            Canvas.SetTop(result, y);

            return result;
        }

        /// <summary>
        /// Calculates the size and location of the rectangle that contains the preview stream within the preview control, when the scaling mode is Uniform
        /// </summary>
        /// <param name="previewResolution">The resolution at which the preview is running</param>
        /// <param name="previewControl">The control that is displaying the preview using Uniform as the scaling mode</param>
        /// <returns></returns>
        public Windows.Foundation.Rect GetPreviewStreamRectInControl(VideoEncodingProperties previewResolution, CaptureElement previewControl)
        {
            var result = new Windows.Foundation.Rect();

            // In case this function is called before everything is initialized correctly, return an empty result
            if (previewControl == null || previewControl.ActualHeight < 1 || previewControl.ActualWidth < 1 ||
                previewResolution == null || previewResolution.Height == 0 || previewResolution.Width == 0)
            {
                return result;
            }

            var streamWidth = previewResolution.Width;
            var streamHeight = previewResolution.Height;

            // For portrait orientations, the width and height need to be swapped
            if (_displayOrientation == DisplayOrientations.Portrait || _displayOrientation == DisplayOrientations.PortraitFlipped)
            {
                streamWidth = previewResolution.Height;
                streamHeight = previewResolution.Width;
            }

            // Start by assuming the preview display area in the control spans the entire width and height both (this is corrected in the next if for the necessary dimension)
            result.Width = previewControl.ActualWidth;
            result.Height = previewControl.ActualHeight;

            // If UI is "wider" than preview, letterboxing will be on the sides
            if ((previewControl.ActualWidth / previewControl.ActualHeight > streamWidth / (double)streamHeight))
            {
                var scale = previewControl.ActualHeight / streamHeight;
                var scaledWidth = streamWidth * scale;

                result.X = (previewControl.ActualWidth - scaledWidth) / 2.0;
                result.Width = scaledWidth;
            }
            else // Preview stream is "wider" than UI, so letterboxing will be on the top+bottom
            {
                var scale = previewControl.ActualWidth / streamWidth;
                var scaledHeight = streamHeight * scale;

                result.Y = (previewControl.ActualHeight - scaledHeight) / 2.0;
                result.Height = scaledHeight;
            }

            return result;
        }
        #endregion

        #region Bluetooth connection, GPS estimated coordinates
        /// <summary>
        /// Start watching for any Bluetooth BLE device. Search for the specific BLE device by finding its address. 
        /// </summary>
        private void StartWatching()
        {
            // Create Bluetooth Listener
            var watcher = new BluetoothLEAdvertisementWatcher
            {
                // Set scanning mode.
                // Active means get all the possible information in the advertisement data.
                // Use Passive if you already know the Ble-Address and only want to connect.
                // Scanning mode Passive is a lot faster.
                ScanningMode = BluetoothLEScanningMode.Active
            };
            // Register callback for when we see an advertisements
            watcher.Received += OnAdvertisementReceivedAsync;
            stopwatch.Start();
            watcher.Start();
        }

        /// <summary>
        /// If bluetooth is connected (Callback is registered) 
        /// </summary>
        /// <param name="watcher"></param>
        /// <param name="eventArgs"></param>
        private async void OnAdvertisementReceivedAsync(BluetoothLEAdvertisementWatcher watcher,
                                                        BluetoothLEAdvertisementReceivedEventArgs eventArgs)
        {
            // Filter for specific Device by name
            if (eventArgs.Advertisement.LocalName == bleDevicName)
            {
                // Stop the watcher 
                watcher.Stop();
                // Read the BLE device information
                var device = await BluetoothLEDevice.FromBluetoothAddressAsync(eventArgs.BluetoothAddress);
                // Check if device is valid (not null) 
                if (device != null)
                {
                    // Register how long it takes to find device
                    deviceFoundMilis = stopwatch.ElapsedMilliseconds;
                    Debug.WriteLine("Device found in " + deviceFoundMilis + " ms");

                    // Register the strength of the BLE device
                    var rssi = eventArgs.RawSignalStrengthInDBm;
                    Debug.WriteLine("Signalstrengt = " + rssi + " DBm");

                    // Register the BLE address of the BLE device
                    var bleAddress = eventArgs.BluetoothAddress;
                    Debug.WriteLine("Ble address = " + bleAddress);

                    // Register the advertistment type 
                    var advertisementType = eventArgs.AdvertisementType;
                    Debug.WriteLine("Advertisement type = " + advertisementType);

                    // Get the GattServices fromt the GUID specified. Find the specific GUID from the BLE module by using a 
                    // BlE finder. 
                    var result = await device.GetGattServicesForUuidAsync(MyService_GUID);

                    // If its sucessful 
                    if (result.Status == GattCommunicationStatus.Success)
                    {
                        // Register how long it takes to connect 
                        connectedMilis = stopwatch.ElapsedMilliseconds;
                        Debug.WriteLine("Connected in " + (connectedMilis - deviceFoundMilis) + " ms");
                        // Check for services
                        var services = result.Services;
                        // Pick the fist services it gets 
                        service = services[0];
                        // If its not null 
                        if (service != null)
                        {
                            // Register how long it takes to have services 
                            serviceFoundMilis = stopwatch.ElapsedMilliseconds;
                            Debug.WriteLine("Service found in " + (serviceFoundMilis - connectedMilis) + " ms");
                            var charResult = await service.GetCharacteristicsForUuidAsync(MYCharacteristic_GUID);
                            if (charResult.Status == GattCommunicationStatus.Success)
                            {
                                charac = charResult.Characteristics[0];
                                if (charac != null)
                                {
                                    characteristicFoundMilis = stopwatch.ElapsedMilliseconds;
                                    Debug.WriteLine("Characteristic found in " +
                                                   (characteristicFoundMilis - serviceFoundMilis) + " ms");

                                    var descriptorValue = GattClientCharacteristicConfigurationDescriptorValue.None;
                                    GattCharacteristicProperties properties = charac.CharacteristicProperties;
                                    string descriptor = string.Empty;

                                    if (properties.HasFlag(GattCharacteristicProperties.Read))
                                    {
                                        Debug.WriteLine("This characteristic supports reading .");
                                    }
                                    if (properties.HasFlag(GattCharacteristicProperties.Write))
                                    {
                                        Debug.WriteLine("This characteristic supports writing .");
                                    }
                                    if (properties.HasFlag(GattCharacteristicProperties.WriteWithoutResponse))
                                    {
                                        Debug.WriteLine("This characteristic supports writing  whithout responce.");
                                    }
                                    if (properties.HasFlag(GattCharacteristicProperties.Notify))
                                    {
                                        descriptor = "notifications";
                                        descriptorValue = GattClientCharacteristicConfigurationDescriptorValue.Notify;
                                        Debug.WriteLine("This characteristic supports subscribing to notifications.");
                                    }
                                    if (properties.HasFlag(GattCharacteristicProperties.Indicate))
                                    {
                                        descriptor = "indications";
                                        descriptorValue = GattClientCharacteristicConfigurationDescriptorValue.Indicate;
                                        Debug.WriteLine("This characteristic supports subscribing to Indication");
                                    }
                                    try
                                    {
                                        var descriptorWriteResult = await charac.WriteClientCharacteristicConfigurationDescriptorAsync(descriptorValue);
                                        if (descriptorWriteResult == GattCommunicationStatus.Success)
                                        {

                                            WriteDescriptorMilis = stopwatch.ElapsedMilliseconds;
                                            Debug.WriteLine("Successfully registered for " + descriptor + " in " + (WriteDescriptorMilis - characteristicFoundMilis) + " ms");
                                            charac.ValueChanged += Charac_ValueChanged; ;
                                        }
                                        else
                                        {
                                            Debug.WriteLine($"Error registering for " + descriptor + ": {result}");
                                            device.Dispose();
                                            device = null;
                                            watcher.Start();//Start watcher again for retry
                                        }
                                    }
                                    catch (UnauthorizedAccessException ex)
                                    {
                                        Debug.WriteLine(ex.Message);
                                    }
                                }
                            }
                            else Debug.WriteLine("No characteristics  found");
                        }
                    }
                    else Debug.WriteLine("No services found");
                }
                else Debug.WriteLine("No device found");
            }
        }
        
        /// <summary>
        /// Receives data from the BLE device. 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="args"></param>
        private static void Charac_ValueChanged(GattCharacteristic sender, GattValueChangedEventArgs args)
        {
            CryptographicBuffer.CopyToByteArray(args.CharacteristicValue, out byte[] data);
            //If data is raw bytes skip all the next lines and use data byte array. Or
            //CryptographicBuffer.CopyToByteArray(args.CharacteristicValue, out byte[] dataArray);
            try
            {
                //Asuming Encoding is in ASCII, can be UTF8 or other!
                dataFromNotify = Encoding.ASCII.GetString(data);
                Debug.Write(dataFromNotify);
            }
            catch (ArgumentException)
            {
                Debug.Write("Unknown format");
            }
        }

        // String from microcontroller, with converted values stored inside double
        string longtitude;  double longtitudenumconv;
        string latitude;    double latitudenumconv;
        string altitude;    double altitudenum;
        string distance;    double distancenum;
        string angle;       double anglenum;
        // X and Y are not in used, can be ignored 
        //string distancex;   double distancexnum;
        //string distancey;   double distanceynum;
        string distancez;   double distanceznum;

        // Variables to be used for Haversine formula for calculation of estimated longitude and latitude
        double fromsealevel;
        double pointlatitude;
        double pointlongtitude;
        double latitudenum;
        double longtitudenum;
        double distancenumkm;

        /// <summary>
        /// Update the textbox based on the last character the string receives. 
        /// The data is sorted and will be displayed based on last character it receives. 
        /// </summary>
        private async void UpdateTextBox()
        {
            while (true)
            {
                // Run the textbox update code on the UI thread
                await Windows.ApplicationModel.Core.CoreApplication.MainView.CoreWindow.Dispatcher.RunAsync(CoreDispatcherPriority.Normal,
                () =>
                {
                    string textdata = dataFromNotify;
                    
                    // Raw distance from LiDAR
                    if (textdata.Contains("W"))
                    {
                        TextBox1.Text = textdata.Remove(textdata.Length - 3);
                        distance = TextBox1.Text;
                        distancenum = Convert.ToDouble(distance);
                        Debug.WriteLine("Raw distance" + distancenum + "\n");
                    }

                    // Raw longtitude value 
                    else if (textdata.Contains("LO"))
                    {
                        TextBox2.Text = textdata.Remove(textdata.Length - 4);
                        longtitude = TextBox2.Text;
                        longtitudenumconv = Convert.ToDouble(longtitude);
                        Debug.WriteLine("Longitude" + longtitude + "\n");
                    }

                    // Raw latitude value 
                    else if (textdata.Contains("LA"))
                    {
                        TextBox3.Text = textdata.Remove(textdata.Length - 4);
                        latitude = TextBox3.Text;
                        latitudenumconv = Convert.ToDouble(latitude);
                        Debug.WriteLine("Latitude" + latitude + "\n");
                    }

                    // Raw angle value relative to North 
                    else if (textdata.Contains("AN"))
                    {
                        //textdata = textdata.Remove(textdata.Length - 1);
                        TextBox4.Text = textdata.Remove(textdata.Length - 4);
                        angle = TextBox4.Text;
                        anglenum = Convert.ToDouble(angle);
                        Debug.WriteLine("Angle" + angle + "\n");
                    }

                    // Raw altitude value 
                    else if (textdata.Contains("AL"))
                    {
                        TextBox5.Text = textdata.Remove(textdata.Length - 4);
                        altitude = TextBox5.Text;
                        altitudenum = Convert.ToDouble(altitude);
                        Debug.WriteLine("Altitude" + altitude + "\n");
                    }

                    /*
                    // X direction distance 
                    else if (textdata.Contains("X"))
                    {
                        distancex = textdata.Remove(textdata.Length - 3);
                        Debug.WriteLine(distancex);
                        distancexnum = Convert.ToDouble(distancex);
                    }

                    // Y direction distance
                    else if (textdata.Contains("Y"))
                    {
                        distancey = textdata.Remove(textdata.Length - 3);
                        Debug.WriteLine(distancey);
                        distanceynum = Convert.ToDouble(distancey);
                    }
                    */

                    // Z direction distance 
                    else if (textdata.Contains("Z"))
                    {
                        distancez = textdata.Remove(textdata.Length - 3);
                        Debug.WriteLine(distancez);
                        distanceznum = Convert.ToDouble(distancez);
                        Debug.WriteLine("Z direction distance" + distancez + "\n");
                    }

                    // Convert latitude and longitude to radians 
                    latitudenum = latitudenumconv * Math.PI / 180;
                    longtitudenum = longtitudenumconv * Math.PI / 180;
                    distancenumkm = distancenum / 1000;
                    // Radius of Earth in KM 
                    int R = 6371;
                    // Convert theta to radians
                    double theta = anglenum * Math.PI / 180;


                    // Refer to this link: https://en.wikipedia.org/wiki/Haversine_formula for calculation of latitude and longitude
                    // Calculate estimated latitude
                    pointlatitude = Math.Asin(Math.Sin(latitudenum) * Math.Cos(distancenumkm / R) + Math.Cos(latitudenum)
                    * Math.Sin(distancenumkm / R) * Math.Cos(theta));

                    // Calculate estimated longitude 
                    pointlongtitude = longtitudenum + Math.Atan2(Math.Sin(theta) * Math.Sin(distancenumkm / R) * Math.Cos(latitudenum),
                    Math.Cos(distancenumkm / R) - Math.Sin(latitudenum) * Math.Sin(pointlatitude));

                    // Convert latitude and longitude back to degrees
                    pointlatitude = pointlatitude * 180 / Math.PI;
                    pointlongtitude = pointlongtitude * 180 / Math.PI;

                    // Display the latitude and longitudde
                    TextBox6.Text = Convert.ToString(pointlongtitude);
                    TextBox7.Text = Convert.ToString(pointlatitude);
                    
                    // Calculate estimated altitude
                    fromsealevel = altitudenum + distanceznum;
                    TextBox8.Text = Convert.ToString(fromsealevel);
                }
                );
            }
        }
        #endregion
    }
}
