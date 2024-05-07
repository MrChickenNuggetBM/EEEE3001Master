#include "main.h"

// Callback for when a message arrives.
void Callback::message_arrived(const_message_ptr msg)
{
    // extract the topic and the payload
    string topic = msg->get_topic();
    string payload = msg->to_string();

    /*
    cout << "Message arrived!" << endl;
    cout << "\ttopic: '" << topic << "'" << endl;
    cout << "\tpayload: '" << payload << "'\n" << endl;
    */

    using namespace topics;
    // define what to do when a particular message is published to the MQTT broker
    // using a particular topic
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
    else if (topic == "parameters/angle")
        parameters::angle = std::stoi(payload);
    else if (topic == "parameters/isGUIControl")
        parameters::isGUIControl = (payload == "true");
    else if (topic == "brightness/isAutomaticBrightness")
        brightness::isAutomaticBrightness = (payload == "true");
    else if (topic == "brightness/dutyCycle")
        brightness::dutyCycle = std::stoi(payload);
    else if (topic == "cv/x-correction")
        cv::xCorrection = std::stoi(payload);
    else if (topic == "cv/y-correction")
        cv::yCorrection = std::stoi(payload);
    else if (topic == "cv/angle-correction")
        cv::angleCorrection = std::stoi(payload);
    else if (topic == "cv/minRad-correction")
        cv::minRadCorrection = std::stoi(payload);
    else if (topic == "cv/majRad-correction")
        cv::majRadCorrection = std::stoi(payload);
    else if (topic == "cv/isNewValues")
        cv::isNewValues = (payload == "true");
    else if (topic == "cv/isPauseRendering")
        cv::isPauseRendering = (payload == "true");
}

bool setup()
{
    // clear the terminal from the cursor
    system("setterm -cursor off;clear");

    // set up the camera
    if (!videoCapture.isOpened())
    {
        cerr << "Error: Could not open camera" << endl;
        return false;
    }

    // set camera image constants
    const float _brightness = 0.55,
                contrast = 0.50,
                saturation = 0.67;


    cout << "brightness: " << _brightness << endl;
    cout << "contrast: " << contrast << endl;
    cout << "saturation: " << saturation << endl;

    // set camera image settings
    videoCapture.set(CAP_PROP_FRAME_WIDTH, 480);
    videoCapture.set(CAP_PROP_FRAME_HEIGHT, 270);
    videoCapture.set(CAP_PROP_BRIGHTNESS, _brightness);
    videoCapture.set(CAP_PROP_CONTRAST, contrast);
    videoCapture.set(CAP_PROP_SATURATION, saturation);

    // set camera settings
    system("v4l2-ctl -c exposure_dynamic_framerate=1");
    system("v4l2-ctl -c scene_mode=8");

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
    // When completed, the callback will subscribe to the topics
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

        auto token = publishMessage("parameters/xCenterSet", to_string(topics::parameters::xCenter));
        token->wait_for(std::chrono::seconds(10));

        token = publishMessage("parameters/yCenterSet", to_string(topics::parameters::yCenter));
        token->wait_for(std::chrono::seconds(10));

        token = publishMessage("parameters/xDiameterSet", to_string(topics::parameters::xDiameter));
        token->wait_for(std::chrono::seconds(10));

        token = publishMessage("parameters/yDiameterSet", to_string(topics::parameters::yDiameter));
        token->wait_for(std::chrono::seconds(10));

        token = publishMessage("parameters/thicknessSet", to_string(topics::parameters::thickness));
        token->wait_for(std::chrono::seconds(10));

        token = publishMessage("parameters/isCircleSet", topics::parameters::isCircle ? "true" : "false");
        token->wait_for(std::chrono::seconds(10));

        token = publishMessage("parameters/modalitySet", to_string(topics::parameters::modality));
        token->wait_for(std::chrono::seconds(10));

        token = publishMessage("parameters/angleSet", to_string(topics::parameters::modality));
        token->wait_for(std::chrono::seconds(10));

        token = publishMessage("parameters/isGUIControlSet", topics::parameters::isGUIControl ? "true" : "false");
        token->wait_for(std::chrono::seconds(10));

        token = publishMessage("cv/isPauseRenderingSet", topics::cv::isPauseRendering ? "true" : "false");
        token->wait_for(std::chrono::seconds(10));

        token = publishMessage("brightness/isAutomaticBrightnessSet", topics::brightness::isAutomaticBrightness ? "true" : "false");
        token->wait_for(std::chrono::seconds(10));

        token = publishMessage("brightness/dutyCycleSet", to_string(topics::brightness::dutyCycle));
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
    using namespace topics;

    // define default parameters
    static int
    _xCenter   = parameters::xCenter * sWidth  / 100, _yCenter = parameters::yCenter * sHeight  / 100,
    _xDiameter = sWidth  * parameters::xDiameter / 100,
    _yDiameter = sHeight * parameters::yDiameter / 100,
    _thickness = parameters::thickness,
    _dutyCycle = brightness::dutyCycle;

    static float _angle     = parameters::angle;

    static bool
    _isCircle  = parameters::isCircle;

    // define default colours for foreground and background
    Scalar
    backgroundColour = Scalar(0, 0, 0, 0),
    ringColour = Scalar(255, 255, 255, 255);

    if (!topics::cv::isPauseRendering)
    {
        // if GUIControl
        // retrieving stored parameters from MQTT
        if (topics::parameters::isGUIControl)
        {
            using namespace topics::parameters;
            // percentage
            _xCenter   = xCenter   * sWidth  / 100;
            _yCenter   = yCenter   * sHeight  / 100;
            _xDiameter = xDiameter * sWidth  / 100;
            _yDiameter = yDiameter * sHeight / 100;
            _thickness = thickness;
            _isCircle  = isCircle;
            _angle     = angle;
        }
        // if new cv parameters from Slave
        // do computer vision -----------------------
        else if (topics::cv::isNewValues)
        {
            using namespace topics::cv;

            // after multiplying by pixel ratio, apply the corrections
            const float pixelRatioW = 0.0853;
            const float pixelRatioH = 0.1;

            _angle     += angleCorrection;
            _xCenter   += (int)(pixelRatioW * xCorrection);
            _yCenter   += (int)(pixelRatioH * yCorrection);
            _xDiameter += (int)(pixelRatioW * majRadCorrection);
            _yDiameter += (int)(pixelRatioH * minRadCorrection);

            topics::cv::isNewValues = false;
        }
        // ------------------------------------------

        // if is (bright/dark)field, fill the circle
        // if darkfield change the colour too
        switch (topics::parameters::modality)
        {
        case 2:
            backgroundColour = Scalar(255, 255, 255, 255);
            ringColour = Scalar(0, 0, 0, 0);
        case 1:
            _thickness = -1;
            break;
        case 0:
            _thickness = parameters::thickness;
            break;
        }
    }

    // if AutomaticBrightness
    // retrieving stored parameters from MQTT
    if (!topics::brightness::isAutomaticBrightness)
        _dutyCycle = topics::brightness::dutyCycle;

    // define the ellipse according to parameters
    Ellipse ellipse(
        Point2f(
            (sWidth / 2) + _xCenter,
            (sHeight / 2) + _yCenter),
        Size2f(
            _isCircle ? std::min(_xDiameter, _yDiameter) : _xDiameter,
            _isCircle ? std::min(_xDiameter, _yDiameter) : _yDiameter),
        _angle,
        ringColour,
        _thickness);

    // get an empty ring image
    Mat ringImage(
        sHeight,
        sWidth,
        CV_8UC4,
        backgroundColour);
    // put the ellipse in ringImage
    ellipse(ringImage);

    // send ring image to Node-RED Dashboard
    auto token = publishImage("images/ring", ringImage);
    token->wait_for(std::chrono::seconds(10));

    // display image
    resize(ringImage, ringImage, Size(1280,2400), INTER_LINEAR);
    screen.send(ringImage);

    // send image plane image to Node-RED Dashboard
    Mat cameraImage;
    videoCapture.read(cameraImage);
    token = publishImage("images/imagePlane", cameraImage);
    token->wait_for(std::chrono::seconds(10));

    // set the duty cycle
    gpioPWM(PWM_PIN, _dutyCycle);

    return (waitKey(1) < 0);
}

void teardown()
{
    // put the cursor back on the CLI
    system("setterm -cursor on");

    cout << "Stopping..." << endl;

    // send a black image to screen
    Mat emptyFrame(1280, 2400, CV_8UC4, Scalar(0, 0, 0, 255));
    screen.send(emptyFrame);

    // disable interfacing to pins
    gpioTerminate();

    // release the camera
    videoCapture.release();

    // close any windows
    // destroyAllWindows();

    // Disconnect from MQTT
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
