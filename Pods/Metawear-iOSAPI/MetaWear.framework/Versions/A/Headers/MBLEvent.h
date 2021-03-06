/**
 * MBLEvent.h
 * MetaWear
 *
 * Created by Stephen Schiffli on 10/8/14.
 * Copyright 2014 MbientLab Inc. All rights reserved.
 *
 * IMPORTANT: Your use of this Software is limited to those specific rights
 * granted under the terms of a software license agreement between the user who
 * downloaded the software, his/her employer (which must be your employer) and
 * MbientLab Inc, (the "License").  You may not use this Software unless you
 * agree to abide by the terms of the License which can be found at
 * www.mbientlab.com/terms . The License limits your use, and you acknowledge,
 * that the  Software may not be modified, copied or distributed and can be used
 * solely and exclusively in conjunction with a MbientLab Inc, product.  Other
 * than for the foregoing purpose, you may not use, reproduce, copy, prepare
 * derivative works of, modify, distribute, perform, display or sell this
 * Software and/or its documentation for any purpose.
 *
 * YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
 * PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
 * NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
 * MBIENTLAB OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT, NEGLIGENCE,
 * STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER LEGAL EQUITABLE
 * THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES INCLUDING BUT NOT LIMITED
 * TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR CONSEQUENTIAL DAMAGES, LOST
 * PROFITS OR LOST DATA, COST OF PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY,
 * SERVICES, OR ANY CLAIMS BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY
 * DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *
 * Should you have any questions regarding your right to use this Software,
 * contact MbientLab Inc, at www.mbientlab.com.
 */

#import <MetaWear/MBLConstants.h>
#import <MetaWear/MBLRegister.h>

@class MBLData;

typedef NS_OPTIONS(uint8_t, MBLComparisonOperation) {
    MBLComparisonOperationEqual = 0,
    MBLComparisonOperationNotEqual = 1,
    MBLComparisonOperationLessThan = 2,
    MBLComparisonOperationLessThanOrEqual = 3,
    MBLComparisonOperationGreaterThan = 4,
    MBLComparisonOperationGreaterThanOrEqual = 5
};

/**
 This object represents "events" generated by sensors and peripherals on the MetaWear board.
 
 There are several things you can do when an event occurs, all of which are programmable using
 this object, they are:
 
 1. Send notifications to the connected iOS device when the event occurs, see
    startNotificationsWithHandler: and stopNotifications
 2. Program other commands to be executed offline on the MetaWear device when the event occurs,
    see programCommandsToRunOnEvent: and eraseCommandsToRunOnEvent.
 3. Log the event in the MetaWear's flash storage, see startLogging and
    downloadLogAndStopLogging:handler:progressHandler:
 4. Pass the event data into a filter, which and process the data in some way and outputs the result
    in a new MBLEvent. See summationOfEvent and periodicSampleOfEvent:.
 
 ## Examples 
 Consider the switch update event, [MBLMechanicalSwitch switchUpdateEvent].
 
**Notifications**:
 
 If you call startNotificationsWithHandler: and keep a live connection to the MetaWear,
 any time you press or release the switch you will get a callback to the provided block.
 
    [device.mechanicalSwitch.switchUpdateEvent startNotificationsWithHandler:^(id obj, NSError *error) {
        // Handle the button press/release
    }];
 
 **Logging**:
 
 If you call startLogging, then anytime you press or release the button, an entry will 
 be created in the log which can be download later using downloadLogAndStopLogging:handler:progressHandler:.
 
    [device.mechanicalSwitch.switchUpdateEvent startLogging];
    // Some time later..
    [device.mechanicalSwitch.switchUpdateEvent downloadLogAndStopLogging:YES/NO handler:{...} progressHandler:{...}];
 
 **Commands**:
 
 If you want the device to buzz when you press the switch then you would do the following:
 
    [device.mechanicalSwitch.switchUpdateEvent programCommandsToRunOnEvent:^{
        [device.hapticBuzzer startHapticWithDutyCycle:248 pulseWidth:500 completion:nil]
    }];
 
 **Processing**:
 
 If you want to log a running count of pushbutton events, you could do the following:
 
    MBLEvent *event = [device.mechanicalSwitch.switchUpdateEvent summationOfEvent];
    [event startLogging];
    // Some time later..
    [event downloadLogAndStopLogging:YES/NO handler:{...} progressHandler:{...}];
 
 ## Gotchas
 
 @warning Calling summationOfEvent or any other filter function returns a freshly created
 MBLEvent object which you must retain for later use.  This is different from the MBLEvent
 properties on the various modules which internally save the MBLEvent object and always return
 the same pointer through the property.
 @warning Since all MBLEvent's are invalidated on disconnect, you need a way to restore your
 custom event on reconnect.  This is where the NSString identifier comes in, you can call
 retrieveEventWithIdentifier: on the freshly connected MBLMetaWear object to get your event back.
 */
@interface MBLEvent : MBLRegister <NSCoding>

///----------------------------------
/// @name Notifications
///----------------------------------

/**
 Start receiving callbacks when this event occurs. The type of the
 object that is passed to the handler depends on the event being handled
 @param handler Block invoked when this event occus
 */
- (void)startNotificationsWithHandler:(MBLObjectHandler)handler;
/**
 Stop receiving callbacks when this event occurs, and release the block provided
 to startNotificationsWithHandler:
 */
- (void)stopNotifications;

///----------------------------------
/// @name Commands
///----------------------------------

/**
 This function is used for programing the Metawear device to perform actions
 automatically.
 
 Any time this even occurs you can have it trigger other
 Metawear API calls even when the phone isn't connected.
 When this function is called, the given block executed and checked for
 validity.  All Metawear API calls inside the block are sent to the device
 for execution later.
 @warning THE BLOCK IS ONLY EXECUTED ONCE DURNING THIS CALL AND
 NEVER AGAIN, DON'T ATTEMPT TO USE CALLBACKS INSIDE THIS BLOCK
 @param block Block consisting of API calls to make when this event occus
 */
- (void)programCommandsToRunOnEvent:(void(^)())block;
/**
 Removes all commands setup when calling programCommandsToRunOnEvent:
 */
- (void)eraseCommandsToRunOnEvent;

///----------------------------------
/// @name Logging
///----------------------------------

/**
 Start recording notifications for this event.
 
 Each time this event occus an entry is made into non-volatile flash memory 
 that is on the metawear device. This is useful for tracking things while the
 phone isn't connected to the Metawear
 */
- (void)startLogging;
/**
 Fetch contents of log from MetaWear device, and optionally turn off logging.
 
 Executes the progressHandler periodically with the progress (0.0 - 1.0),
 progressHandler will get called with 1.0 before handler is called.  The handler 
 is passed an array of entries, the exact class of the entry depends on what is 
 being logged.  For example, the accelerometer log returns an array of MBLAccelerometerData's
 @param stopLogging YES: Stop logging the current event, NO: Keep logging the event after download
 @param handler Callback once download is complete
 @param progressHandler Periodically called while log download is in progress
 */
- (void)downloadLogAndStopLogging:(BOOL)stopLogging
                          handler:(MBLArrayErrorHandler)handler
                  progressHandler:(MBLFloatHandler)progressHandler;
/**
 See if this event is currently being logged
 @returns YES if logging, NO otherwise
 */
- (BOOL)isLogging;

///----------------------------------
/// @name Processing Filters
///----------------------------------

/**
 Create a new event that accumulates the output data of the current event.
 @returns New event representing accumulated output
 */
- (MBLEvent *)summationOfEvent;

/**
 Create a new event that occurs at most once every period milliseconds.
 @param periodInMsec Sample period in msec
 @returns New event representing periodically sampled output
 */
- (MBLEvent *)periodicSampleOfEvent:(uint32_t)periodInMsec;

/**
 Create a new event that occurs at the same time of this event, but whose value is
 read from the data object passed in.
 @param data Object to be read when this event occurs
 @returns New event representing the data read
 */
- (MBLEvent *)readDataOnEvent:(MBLData *)data;

///----------------------------------
/// @name Deprecated Functions
///----------------------------------

/**
 * @deprecated create an id<MBLRestorable> object and use [MBLMetaWear setConfiguration:handler:] instead
 */
- (MBLEvent *)periodicSampleOfEvent:(uint32_t)periodInMsec identifier:(NSString *)identifier DEPRECATED_MSG_ATTRIBUTE("Create an id<MBLRestorable> object and use [MBLMetaWear setConfiguration:handler:] instead");

/**
 * @deprecated create an id<MBLRestorable> object and use [MBLMetaWear setConfiguration:handler:] instead
 */
- (MBLEvent *)summationOfEventWithIdentifier:(NSString *)identifier DEPRECATED_MSG_ATTRIBUTE("Create an id<MBLRestorable> object and use [MBLMetaWear setConfiguration:handler:] instead");

@end
