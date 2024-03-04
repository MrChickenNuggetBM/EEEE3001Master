#include "main.h"

// Callback for when a message arrives.
void Callback::message_arrived(const_message_ptr msg)
{
    using namespace topics;

    string topic = msg->get_topic();
    string payload = msg->to_string();

    /*
    cout << "Message arrived!" << endl;
    cout << "\ttopic: '" << topic << "'" << endl;
    cout << "\tpayload: '" << payload << "'\n" << endl;
    */

    if (topic == "parameters/xCenter")
        parameters::xCenter = std::stoi(payload);
    else if (topic == "parameters/yCenter")
        parameters::yCenter = std::stoi(payload);
    else if (topic == "parameters/xDiameter")
        parameters::xDiameter = std::stoi(payload);
    else if (topic == "parameters/yDiameter")
        parameters::yDiameter = std::stoi(payload);
    else if (topic == "parameters/thickness")
        parameters::thickness = std::stoi(payload);
    else if (topic == "parameters/isCircle")
        parameters::isCircle = (payload == "true");
    else if (topic == "parameters/modality")
        parameters::modality = std::stoi(payload);
    else if (topic == "parameters/isGUIControl")
        parameters::isGUIControl = (payload == "true");
    else if (topic == "brightness/isAutomaticBrightness")
        brightness::isAutomaticBrightness = (payload == "true");
    else if (topic == "brightness/dutyCycle")
        brightness::dutyCycle = std::stoi(payload);
}

bool setup()
{
    // clear the terminal
    system("setterm -cursor off;clear");

    // set up the camera
    if (!videoCapture.isOpened())
    {
        cerr << "Error: Could not open camera" << endl;
        return false;
    }

    videoCapture.set(CAP_PROP_FRAME_WIDTH, 960);
    videoCapture.set(CAP_PROP_FRAME_HEIGHT, 540);

    // set the PWM signal
    if (gpioInitialise() < 0)
        return false;
    gpioSetMode(PWM_PIN, PI_OUTPUT);
    gpioSetPWMfrequency(PWM_PIN, 1000);
    gpioSetPWMrange(PWM_PIN, 100);
    gpioPWM(PWM_PIN, 50);

    // configure code termination
    atexit(teardown);
    signal(SIGINT, teardown);

    // establish broker-client connection

    OPTIONS.set_clean_session(false);

    // Install the callback(s) before connecting.

    CLIENT.set_callback(CALLBACK);

    // Start the connection.
    // When completed, the callback will subscribe to topic.
    try
    {
        cout << "Connecting to the MQTT server..." << flush;
        auto connectToken = CLIENT.connect(OPTIONS, nullptr, CALLBACK);
        connectToken->wait_for(std::chrono::seconds(10));
    }
    catch (const mqtt::exception &exc)
    {
        cerr << "\nERROR: Unable to connect to MQTT server: '"
             << SERVER_ADDRESS << "'" << exc << endl;
        return false;
    }

    // publishing the default values
    try
    {
        using namespace topics::parameters;
        auto token = publishMessage("parameters/xCenterSet", "0", CLIENT);
        token->wait_for(std::chrono::seconds(10));

        token = publishMessage("parameters/yCenterSet", "0", CLIENT);
        token->wait_for(std::chrono::seconds(10));

        token = publishMessage("parameters/xDiameterSet", "33", CLIENT);
        token->wait_for(std::chrono::seconds(10));

        token = publishMessage("parameters/yDiameterSet", "60", CLIENT);
        token->wait_for(std::chrono::seconds(10));

        token = publishMessage("parameters/thicknessSet", "150", CLIENT);
        token->wait_for(std::chrono::seconds(10));

        token = publishMessage("parameters/isCircleSet", "false", CLIENT);
        token->wait_for(std::chrono::seconds(10));

        token = publishMessage("parameters/modalitySet", "0", CLIENT);
        token->wait_for(std::chrono::seconds(10));

        token = publishMessage("parameters/isGUIControlSet", "false", CLIENT);
        token->wait_for(std::chrono::seconds(10));

        token = publishMessage("brightness/isAutomaticBrightnessSet", "true", CLIENT);
        token->wait_for(std::chrono::seconds(10));

        token = publishMessage("brightness/dutyCycleSet", "50", CLIENT);
        token->wait_for(std::chrono::seconds(10));
    }
    catch (const mqtt::exception &exc)
    {
        std::cerr << "Error during publish" << exc.what() << std::endl;
    }

    return true;
}

bool loop()
{
    // do computer vision -----------------------
    Mat cameraImage;
    videoCapture.read(cameraImage);

    // replace default values
    int _xCenter = 0, _yCenter = 0,
        _xDiameter = sWidth * 0.33,
        _yDiameter = sHeight * 0.6,
        _thickness = 150,
        _dutyCycle = 50;
    bool _isCircle = false;
    // ------------------------------------------

    // if GUIControl
    // retrieving stored parameters from MQTT
    if (topics::parameters::isGUIControl)
    {
        using namespace topics::parameters;
        // percentage
        _xCenter = xCenter * sWidth / 100;
        _yCenter = yCenter * sHeight / 100;
        _xDiameter = xDiameter * sWidth / 100;
        _yDiameter = yDiameter * sHeight / 100;
        _thickness = thickness;
        _isCircle = isCircle;
    }

    // if AutomaticBrightness
    // retrieving stored parameters from MQTT
    if (!topics::brightness::isAutomaticBrightness)
        _dutyCycle = topics::brightness::dutyCycle;

    // define default colours
    Scalar backgroundColour = Scalar(0, 0, 0, 0);
    Scalar ringColour = Scalar(255, 255, 255, 255);

    // if is (bright/dark)field, fill the circle
    // if darkfield change the colour too
    switch (topics::parameters::modality)
    {
    case 2:
        backgroundColour = Scalar(255, 255, 255, 255);
        ringColour = Scalar(0, 0, 0, 0);
    case 1:
        _thickness = -1;
    case 0:
        break;
    }

    // define the ellipse
    Ellipse ellipse(
        Point2f(
            960 + _xCenter,
            540 + _yCenter),
        Size2f(
            _isCircle ? std::min(_xDiameter, _yDiameter) : _xDiameter,
            _isCircle ? std::min(_xDiameter, _yDiameter) : _yDiameter),
        0,
        ringColour,
        _thickness);

    // define the ringImage frame
    Mat ringImage(
        1080,
        1920,
        CV_8UC4,
        backgroundColour);
    // put the ellipse in ringImage
    ellipse(ringImage);

    // display image
    screen.send(ringImage);
    // send ring image to Node-RED Dashboard
    auto token = publishImage("images/ring", ringImage, CLIENT);
    token->wait_for(std::chrono::seconds(10));

    // send bfp image to Node-RED Dashboard
    token = publishImage("images/imagePlane", cameraImage, CLIENT);
    token->wait_for(std::chrono::seconds(10));

    // detect two ellipses on the image (outer and inner ring)
    // vector<Ellipse> ellipses = detectEllipses(cameraImage.clone(), 2);
    // cout << "I found: " << ellipses.size() << endl;

    // set the duty cycle
    gpioPWM(PWM_PIN, _dutyCycle);

    return (waitKey(1) < 0);
}

void teardown()
{
    system("setterm -cursor on");

    cout << "Stopping..." << endl;

    Mat emptyFrame(1080, 1920, CV_8UC4, Scalar(0, 0, 0, 255));
    screen.send(emptyFrame);

    gpioTerminate();

    videoCapture.release();
    destroyAllWindows();

    // Disconnect
    try
    {
        cout << "\nDisconnecting from the MQTT server..." << flush;
        CLIENT.disconnect()->wait();
        cout << "OK" << endl;
    }
    catch (const mqtt::exception &exc)
    {
        cerr << exc << endl;
    }

    cout << endl
         << "Stopped after " << i << " frames" << endl;
}
