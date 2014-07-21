// Copyright (C) by Ashton Mason. See LICENSE.txt for licensing information.
#ifndef THERON_TESTS_TESTSUITES_NETWORKTESTSUITE_H
#define THERON_TESTS_TESTSUITES_NETWORKTESTSUITE_H


#include <string.h>

#include <Theron/Theron.h>
#include <Theron/Detail/Threading/Utils.h>

#include "TestFramework/TestSuite.h"


#if !THERON_XS
#error This test suite requires network support!
#endif


namespace Tests
{


class NetworkTestSuite : public TestFramework::TestSuite
{
public:

    inline NetworkTestSuite()
    {
        TESTFRAMEWORK_REGISTER_TESTSUITE(NetworkTestSuite);

        TESTFRAMEWORK_REGISTER_TEST(SendMessageToRemoteActorByName);
        TESTFRAMEWORK_REGISTER_TEST(SendMessageToRemoteActorFromNullAddress);
		TESTFRAMEWORK_REGISTER_TEST(SendEmptyMessageToRemoteActor);
    }

    inline static void SendMessageToRemoteActorByName()
    {
        typedef Replier<RemoteMessage> RemoteMessageReplier;
        typedef Theron::Catcher<RemoteMessage> RemoteMessageCatcher;

        // Create two local network endpoints.
        Theron::EndPoint endPointOne("one", "inproc://endpoint_one");
        Theron::EndPoint endPointTwo("two", "inproc://endpoint_two");

        // Connect the endpoints.
        endPointOne.Connect("inproc://endpoint_two");
        endPointTwo.Connect("inproc://endpoint_one");

        // Create a framework and receiver at endpoint one.
        Theron::Framework frameworkOne(endPointOne);
        Theron::Receiver receiver(endPointOne, "receiver");

        // Create a replier in a framework at endpoint two.
        Theron::Framework frameworkTwo(endPointTwo);
        RemoteMessageReplier replier(frameworkTwo, "replier");

        // Register a catcher with the receiver.
        RemoteMessageCatcher catcher;
        receiver.RegisterHandler(&catcher, &RemoteMessageCatcher::Push);

        RemoteMessage message;
        message.mData[0] = 'H';
        message.mData[1] = 'e';
        message.mData[2] = 'l';
        message.mData[3] = 'l';
        message.mData[4] = 'o';
        message.mData[5] = '\0';

        // We have to resend until the Connect actually connects.
        // Send the message and wait for the reply.
        while (receiver.Count() == 0)
        {
            // We send the replier a message by name, and pass the receiver address by name.
            frameworkOne.Send(
                message,
                Theron::Address("receiver"),
                Theron::Address("replier"));

            Theron::Detail::Utils::SleepThread(100);
        }

        receiver.Wait();

        RemoteMessage caught;
        Theron::Address from;
        Check(!catcher.Empty(), "Failed to catch remote message");
        Check(catcher.Pop(caught, from), "Failed to pop caught remote message");

        Check(strcmp(caught.mData, "Hello") == 0, "Remote message has bad value");
        Check(from == Theron::Address("replier"), "Remote message from-address is wrong");
        Check(from == replier.GetAddress(), "Remote message from-address is wrong");
    }

    inline static void SendMessageToRemoteActorFromNullAddress()
    {
        typedef Notifier<RemoteMessage> RemoteMessageNotifier;
        typedef Theron::Catcher<RemoteMessage> RemoteMessageCatcher;

        // Create two local network endpoints.
        Theron::EndPoint endPointOne("one", "inproc://endpoint_one");
        Theron::EndPoint endPointTwo("two", "inproc://endpoint_two");

        // Connect the endpoints.
        endPointOne.Connect("inproc://endpoint_two");
        endPointTwo.Connect("inproc://endpoint_one");

        // Create a framework and receiver at endpoint one.
        Theron::Framework frameworkOne(endPointOne);
        Theron::Receiver receiver(endPointOne, "receiver");

        // Create a notifier in a framework at endpoint two.
        Theron::Framework frameworkTwo(endPointTwo);
        RemoteMessageNotifier notifier(Theron::Address("receiver"), frameworkTwo, "notifier");

        // Register a catcher with the receiver.
        RemoteMessageCatcher catcher;
        receiver.RegisterHandler(&catcher, &RemoteMessageCatcher::Push);

        RemoteMessage message;
        message.mData[0] = 'H';
        message.mData[1] = 'e';
        message.mData[2] = 'l';
        message.mData[3] = 'l';
        message.mData[4] = 'o';
        message.mData[5] = '\0';

        // We have to resend until the Connect actually connects.
        // Send the message and wait for the reply.
        while (receiver.Count() == 0)
        {
            // Send the notifier a message from a null 'from' address.
            frameworkOne.Send(
                message,
                Theron::Address(),
                Theron::Address("notifier"));

            Theron::Detail::Utils::SleepThread(100);
        }

        receiver.Wait();

        RemoteMessage caught;
        Theron::Address from;
        Check(!catcher.Empty(), "Failed to catch remote message");
        Check(catcher.Pop(caught, from), "Failed to pop caught remote message");

        Check(strcmp(caught.mData, "Hello") == 0, "Remote message has bad value");
        Check(from == Theron::Address("notifier"), "Remote message from-address is wrong");
        Check(from == notifier.GetAddress(), "Remote message from-address is wrong");
    }

	inline static void SendEmptyMessageToRemoteActor()
	{
		typedef Replier<EmptyMessage> EmptyMessageReplier;
		typedef Theron::Catcher<EmptyMessage> EmptyMessageCatcher;

		// Create two local network endpoints.
		Theron::EndPoint endPointOne("one", "inproc://endpoint_one");
		Theron::EndPoint endPointTwo("two", "inproc://endpoint_two");

		// Connect the endpoints.
		endPointOne.Connect("inproc://endpoint_two");
		endPointTwo.Connect("inproc://endpoint_one");

		// Create a framework and receiver at endpoint one.
		Theron::Framework frameworkOne(endPointOne);
		Theron::Receiver receiver(endPointOne, "receiver");

		// Create a replier in a framework at endpoint two.
		Theron::Framework frameworkTwo(endPointTwo);
		EmptyMessageReplier replier(frameworkTwo, "replier");

		// Register a catcher with the receiver.
		EmptyMessageCatcher catcher;
		receiver.RegisterHandler(&catcher, &EmptyMessageCatcher::Push);

		EmptyMessage message;

		// We have to resend until the Connect actually connects.
		// Send the message and wait for the reply.
		while (receiver.Count() == 0)
		{
			// We send the replier a message by name, and pass the receiver address by name.
			frameworkOne.Send(
				message,
				Theron::Address("receiver"),
				Theron::Address("replier"));

			Theron::Detail::Utils::SleepThread(100);
		}

		receiver.Wait();

		EmptyMessage caught;
		Theron::Address from;
		Check(!catcher.Empty(), "Failed to catch empty message");
		Check(catcher.Pop(caught, from), "Failed to pop caught empty message");

		Check(from == Theron::Address("replier"), "Remote message from-address is wrong");
		Check(from == replier.GetAddress(), "Remote message from-address is wrong");
	}

private:

    template <class MessageType>
    class Replier : public Theron::Actor
    {
    public:

        inline explicit Replier(Theron::Framework &framework, const char *const name = 0) : Theron::Actor(framework, name)
        {
            RegisterHandler(this, &Replier::Handler);
        }

    private:

        inline void Handler(const MessageType &message, const Theron::Address from)
        {
            Send(message, from);
        }
    };

    template <class MessageType>
    class Notifier : public Theron::Actor
    {
    public:

        inline explicit Notifier(const Theron::Address &notify, Theron::Framework &framework, const char *const name = 0) :
          Theron::Actor(framework, name),
          mNotify(notify)
        {
            RegisterHandler(this, &Notifier::Handler);
        }

    private:

        inline void Handler(const MessageType &message, const Theron::Address /*from*/)
        {
            Send(message, mNotify);
        }

        Theron::Address mNotify;
    };

    struct RemoteMessage
    {
        char mData[8];
    };

	struct EmptyMessage
	{
	};
};


} // namespace Tests


THERON_REGISTER_MESSAGE(Tests::NetworkTestSuite::RemoteMessage);
THERON_REGISTER_MESSAGE(Tests::NetworkTestSuite::EmptyMessage);


#endif // THERON_TESTS_TESTSUITES_NETWORKTESTSUITE_H
