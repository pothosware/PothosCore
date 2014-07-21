// Copyright (C) by Ashton Mason. See LICENSE.txt for licensing information.
#ifndef THERON_TESTS_TESTSUITES_FEATURETESTSUITE_H
#define THERON_TESTS_TESTSUITES_FEATURETESTSUITE_H


#include <string.h>
#include <vector>
#include <string>
#include <queue>

#include <Theron/Theron.h>

#include <Theron/Detail/Threading/Utils.h>

#include "TestFramework/TestSuite.h"


namespace Tests
{


class FeatureTestSuite : public TestFramework::TestSuite
{
public:

    inline FeatureTestSuite()
    {
        TESTFRAMEWORK_REGISTER_TESTSUITE(FeatureTestSuite);

        TESTFRAMEWORK_REGISTER_TEST(ConstructFramework);
        TESTFRAMEWORK_REGISTER_TEST(ConstructFrameworkThreadCount);
        TESTFRAMEWORK_REGISTER_TEST(ConstructFrameworkDefaultParams);
        TESTFRAMEWORK_REGISTER_TEST(ConstructFrameworkParamsThreadCount);
        TESTFRAMEWORK_REGISTER_TEST(ConstructFrameworkParamsNodeProcessor);
        TESTFRAMEWORK_REGISTER_TEST(ConstructFrameworkParamsStrategy);
        TESTFRAMEWORK_REGISTER_TEST(ConstructFrameworkParamsPriorityOne);
        TESTFRAMEWORK_REGISTER_TEST(ConstructFrameworkParamsPriorityMinusOne);
        TESTFRAMEWORK_REGISTER_TEST(ConstructActor);
        TESTFRAMEWORK_REGISTER_TEST(ConstructMultipleActors);
        TESTFRAMEWORK_REGISTER_TEST(ConstructAddress);
        TESTFRAMEWORK_REGISTER_TEST(CopyAddress);
        TESTFRAMEWORK_REGISTER_TEST(AssignAddress);
        TESTFRAMEWORK_REGISTER_TEST(GetActorFramework);
        TESTFRAMEWORK_REGISTER_TEST(GetActorAddress);
        TESTFRAMEWORK_REGISTER_TEST(RegisterHandler);
        TESTFRAMEWORK_REGISTER_TEST(SendHandledMessageInBlockingFramework);
        TESTFRAMEWORK_REGISTER_TEST(SendHandledMessageInNonBlockingFramework);
        TESTFRAMEWORK_REGISTER_TEST(CreateActorInFunction);
        TESTFRAMEWORK_REGISTER_TEST(SendMessageToReceiverInFunction);
        TESTFRAMEWORK_REGISTER_TEST(SendMessageFromNullAddressInFunction);
        TESTFRAMEWORK_REGISTER_TEST(SendMessageToActorFromNullAddressInFunction);
        TESTFRAMEWORK_REGISTER_TEST(SendMessageToActorFromReceiverInFunction);
        TESTFRAMEWORK_REGISTER_TEST(ReceiveReplyInFunction);
        TESTFRAMEWORK_REGISTER_TEST(CatchReplyInFunction);
        TESTFRAMEWORK_REGISTER_TEST(SendNonPODMessageInFunction);
        TESTFRAMEWORK_REGISTER_TEST(SendPointerMessageInFunction);
        TESTFRAMEWORK_REGISTER_TEST(SendConstPointerMessageInFunction);
        TESTFRAMEWORK_REGISTER_TEST(CreateDerivedActor);
        TESTFRAMEWORK_REGISTER_TEST(SendMessageToDerivedActor);
        TESTFRAMEWORK_REGISTER_TEST(IncrementCounter);
        TESTFRAMEWORK_REGISTER_TEST(ActorTemplate);
        TESTFRAMEWORK_REGISTER_TEST(OneHandlerAtATime);
        TESTFRAMEWORK_REGISTER_TEST(MultipleHandlersForMessageType);
        TESTFRAMEWORK_REGISTER_TEST(MessageArrivalOrder);
        TESTFRAMEWORK_REGISTER_TEST(SendAddressAsMessage);
        TESTFRAMEWORK_REGISTER_TEST(SendMessageToDefaultHandlerInFunction);
        TESTFRAMEWORK_REGISTER_TEST(RegisterHandlerFromHandler);
        TESTFRAMEWORK_REGISTER_TEST(CreateActorInConstructor);
        TESTFRAMEWORK_REGISTER_TEST(SendMessageInConstructor);
        TESTFRAMEWORK_REGISTER_TEST(DeregisterHandlerInConstructor);
        TESTFRAMEWORK_REGISTER_TEST(CreateActorInDestructor);
        TESTFRAMEWORK_REGISTER_TEST(SendMessageInDestructor);
        TESTFRAMEWORK_REGISTER_TEST(DeregisterHandlerInDestructor);
        TESTFRAMEWORK_REGISTER_TEST(CreateActorInHandler);
        TESTFRAMEWORK_REGISTER_TEST(GetNumQueuedMessagesInHandler);
        TESTFRAMEWORK_REGISTER_TEST(GetNumQueuedMessagesInFunction);
        TESTFRAMEWORK_REGISTER_TEST(UseBlindDefaultHandler);
        TESTFRAMEWORK_REGISTER_TEST(IsHandlerRegisteredInHandler);
        TESTFRAMEWORK_REGISTER_TEST(SetFallbackHandler);
        TESTFRAMEWORK_REGISTER_TEST(HandleUndeliveredMessageSentInFunction);
        TESTFRAMEWORK_REGISTER_TEST(HandleUnhandledMessageSentInFunction);
        TESTFRAMEWORK_REGISTER_TEST(HandleUndeliveredBlindMessageSentInFunction);
        TESTFRAMEWORK_REGISTER_TEST(HandleMessageSentToStaleFrameworkInFunction);
        TESTFRAMEWORK_REGISTER_TEST(HandleMessageSentToStaleFrameworkInHandler);
        TESTFRAMEWORK_REGISTER_TEST(SendRegisteredMessage);
        TESTFRAMEWORK_REGISTER_TEST(DeriveFromActorFirst);
        TESTFRAMEWORK_REGISTER_TEST(DeriveFromActorLast);
        TESTFRAMEWORK_REGISTER_TEST(SendEmptyMessage);
        TESTFRAMEWORK_REGISTER_TEST(MultipleFrameworks);
        TESTFRAMEWORK_REGISTER_TEST(ConstructFrameworkWithParameters);
        TESTFRAMEWORK_REGISTER_TEST(ThreadCountApi);
        TESTFRAMEWORK_REGISTER_TEST(EventCounterApi);
        TESTFRAMEWORK_REGISTER_TEST(ConstructEndPoint);
        TESTFRAMEWORK_REGISTER_TEST(TieFrameworkToEndPoint);
        TESTFRAMEWORK_REGISTER_TEST(TieActorsToEndPoint);
        TESTFRAMEWORK_REGISTER_TEST(TieReceiverstoEndPoint);
        TESTFRAMEWORK_REGISTER_TEST(NameFrameworkOnConstruction);
        TESTFRAMEWORK_REGISTER_TEST(NameActorOnConstruction);
        TESTFRAMEWORK_REGISTER_TEST(NameReceiverOnConstruction);
        TESTFRAMEWORK_REGISTER_TEST(SendMessageToLocalActorByName);
        TESTFRAMEWORK_REGISTER_TEST(SendMessagesBetweenLocalFrameworksByName);
    }

    inline static void ConstructFramework()
    {
        Theron::Framework framework;
    }

    inline static void ConstructFrameworkThreadCount()
    {
        Theron::Framework framework(16);
    }

    inline static void ConstructFrameworkDefaultParams()
    {
        Theron::Framework::Parameters params;
        Theron::Framework framework(params);
    }

    inline static void ConstructFrameworkParamsThreadCount()
    {
        Theron::Framework::Parameters params(16);
        Theron::Framework framework(params);
    }

    inline static void ConstructFrameworkParamsNodeProcessor()
    {
        Theron::Framework::Parameters params(16, 0x1, 0xFFFF);
        params.mNodeMask = 0x1;
        params.mProcessorMask = 0xFFFF;
        Theron::Framework framework(params);
    }

    inline static void ConstructFrameworkParamsStrategy()
    {
        Theron::Framework::Parameters params(16, 0x1, 0xFFFF, Theron::YIELD_STRATEGY_CONDITION);
        params.mYieldStrategy = Theron::YIELD_STRATEGY_CONDITION;
        Theron::Framework framework(params);
    }

    inline static void ConstructFrameworkParamsPriorityOne()
    {
        Theron::Framework::Parameters params(16, 0x1, 0xFFFF, Theron::YIELD_STRATEGY_CONDITION, 1.0f);
        params.mThreadPriority = 1.0f;
        Theron::Framework framework(params);
    }

    inline static void ConstructFrameworkParamsPriorityMinusOne()
    {
        Theron::Framework::Parameters params(16, 0x1, 0xFFFF, Theron::YIELD_STRATEGY_CONDITION, -1.0f);
        params.mThreadPriority = -1.0f;
        Theron::Framework framework(params);
    }

    inline static void ConstructActor()
    {
        Theron::Framework framework;
        TrivialActor actor(framework);
    }

    inline static void ConstructMultipleActors()
    {
        Theron::Framework framework;
        TrivialActor actor0(framework);
        TrivialActor actor1(framework);
    }

    inline static void ConstructAddress()
    {
        Theron::Address address;
        Check(address == Theron::Address::Null(), "Default-constructed address should be null");
    }

    inline static void CopyAddress()
    {
        Theron::Framework framework;
        TrivialActor actor(framework);
        const Theron::Address address(actor.GetAddress());
        const Theron::Address copied(address);

        Check(address != Theron::Address::Null(), "Actor address should be null");
        Check(copied != Theron::Address::Null(), "Copied actor address should be null");
        Check(copied == address, "Address not copied correctly");
    }

    inline static void AssignAddress()
    {
        Theron::Framework framework;
        TrivialActor actor(framework);
        const Theron::Address address(actor.GetAddress());
        Theron::Address assigned;

        Check(address != Theron::Address::Null(), "Actor address should be null");

        assigned = address;
        Check(assigned != Theron::Address::Null(), "Copied actor address should be null");
        Check(assigned == address, "Address not copied correctly");
    }

    inline static void GetActorFramework()
    {
        Theron::Framework framework;
        TrivialActor actor(framework);
        Theron::Framework &actorFramework(actor.GetFramework());
        Check(&actorFramework == &framework, "Actor framework incorrect");
    }

    inline static void GetActorAddress()
    {
        Theron::Framework framework;
        TrivialActor actor(framework);
        const Theron::Address address(actor.GetAddress());
        Check(address != Theron::Address::Null(), "Actor address should not be null");
    }

    inline static void RegisterHandler()
    {
        Theron::Framework framework;
        Registrar<int> actor(framework);
    }

    inline static void SendHandledMessageInBlockingFramework()
    {
        Theron::Framework::Parameters params;
        params.mYieldStrategy = Theron::YIELD_STRATEGY_CONDITION;

        Theron::Framework framework(params);
        Theron::Receiver receiver;
        Replier<int> actor(framework);

        framework.Send(int(0), receiver.GetAddress(), actor.GetAddress());
        framework.Send(int(1), receiver.GetAddress(), actor.GetAddress());
        framework.Send(int(2), receiver.GetAddress(), actor.GetAddress());

        receiver.Wait();
        receiver.Wait();
        receiver.Wait();
    }

    inline static void SendHandledMessageInNonBlockingFramework()
    {
        Theron::Framework::Parameters params;
        params.mYieldStrategy = Theron::YIELD_STRATEGY_HYBRID;

        Theron::Framework framework(params);
        Theron::Receiver receiver;
        Replier<int> actor(framework);

        framework.Send(int(0), receiver.GetAddress(), actor.GetAddress());
        framework.Send(int(1), receiver.GetAddress(), actor.GetAddress());
        framework.Send(int(2), receiver.GetAddress(), actor.GetAddress());

        receiver.Wait();
        receiver.Wait();
        receiver.Wait();
    }

    inline static void CreateActorInFunction()
    {
        Theron::Framework framework;
        TrivialActor actor(framework);
    }

    inline static void SendMessageToReceiverInFunction()
    {
        Theron::Framework framework;
        Theron::Receiver receiver;
        framework.Send(0.0f, receiver.GetAddress(), receiver.GetAddress());
        receiver.Wait();
    }

    inline static void SendMessageFromNullAddressInFunction()
    {
        Theron::Framework framework;
        Theron::Receiver receiver;

        framework.Send(0, Theron::Address::Null(), receiver.GetAddress());
        receiver.Wait();
    }

    inline static void SendMessageToActorFromNullAddressInFunction()
    {
        Theron::Framework framework;
        Theron::Receiver receiver;
        Signaller signaller(framework);

        framework.Send(receiver.GetAddress(), Theron::Address::Null(), signaller.GetAddress());
        receiver.Wait();
    }

    inline static void SendMessageToActorFromReceiverInFunction()
    {
        Theron::Framework framework;
        Theron::Receiver receiver;
        Signaller signaller(framework);

        framework.Send(receiver.GetAddress(), receiver.GetAddress(), signaller.GetAddress());
        receiver.Wait();
    }

    inline static void ReceiveReplyInFunction()
    {
        typedef Replier<float> FloatReplier;

        Theron::Framework framework;
        Theron::Receiver receiver;
        FloatReplier actor(framework);

        framework.Send(5.0f, receiver.GetAddress(), actor.GetAddress());

        receiver.Wait();
    }

    inline static void CatchReplyInFunction()
    {
        typedef Replier<float> FloatReplier;
        typedef Catcher<float> FloatCatcher;

        Theron::Framework framework;
        FloatReplier actor(framework);

        Theron::Receiver receiver;
        FloatCatcher catcher;
        receiver.RegisterHandler(&catcher, &FloatCatcher::Catch);

        framework.Send(5.0f, receiver.GetAddress(), actor.GetAddress());

        receiver.Wait();

        Check(catcher.mMessage == 5.0f, "Caught message value wrong");
        Check(catcher.mFrom == actor.GetAddress(), "Caught from address wrong");
    }

    inline static void SendNonPODMessageInFunction()
    {
        typedef std::vector<int> VectorMessage;
        typedef Replier<VectorMessage> VectorReplier;
        typedef Catcher<VectorMessage> VectorCatcher;

        Theron::Framework framework;
        VectorReplier actor(framework);

        Theron::Receiver receiver;
        VectorCatcher catcher;
        receiver.RegisterHandler(&catcher, &VectorCatcher::Catch);

        VectorMessage vectorMessage;
        vectorMessage.push_back(0);
        vectorMessage.push_back(1);
        vectorMessage.push_back(2);
        framework.Send(vectorMessage, receiver.GetAddress(), actor.GetAddress());

        receiver.Wait();

        Check(catcher.mMessage == vectorMessage, "Reply message is wrong");
    }

    inline static void SendPointerMessageInFunction()
    {
        typedef float * PointerMessage;
        typedef Replier<PointerMessage> PointerReplier;
        typedef Catcher<PointerMessage> PointerCatcher;

        Theron::Framework framework;
        PointerReplier actor(framework);

        Theron::Receiver receiver;
        PointerCatcher catcher;
        receiver.RegisterHandler(&catcher, &PointerCatcher::Catch);

        float a(0.0f);
        PointerMessage pointerMessage(&a);
        framework.Send(pointerMessage, receiver.GetAddress(), actor.GetAddress());

        receiver.Wait();

        Check(catcher.mMessage == &a, "Reply message is wrong");
    }

    inline static void SendConstPointerMessageInFunction()
    {
        typedef const float * PointerMessage;
        typedef Replier<PointerMessage> PointerReplier;
        typedef Catcher<PointerMessage> PointerCatcher;

        Theron::Framework framework;
        PointerReplier actor(framework);

        Theron::Receiver receiver;
        PointerCatcher catcher;
        receiver.RegisterHandler(&catcher, &PointerCatcher::Catch);

        float a(0.0f);
        PointerMessage pointerMessage(&a);
        framework.Send(pointerMessage, receiver.GetAddress(), actor.GetAddress());

        receiver.Wait();

        Check(catcher.mMessage == &a, "Reply message is wrong");
    }

    inline static void CreateDerivedActor()
    {
        Theron::Framework framework;
        StringReplier actor(framework);
    }

    inline static void SendMessageToDerivedActor()
    {
        typedef const char * StringMessage;
        typedef Catcher<StringMessage> StringCatcher;

        Theron::Framework framework;
        StringReplier actor(framework);
    
        Theron::Receiver receiver;
        StringCatcher catcher;
        receiver.RegisterHandler(&catcher, &StringCatcher::Catch);
    
        const char *testString = "hello";
        StringMessage stringMessage(testString);
        framework.Send(stringMessage, receiver.GetAddress(), actor.GetAddress());

        receiver.Wait();

        Check(catcher.mMessage == stringMessage, "Reply message is wrong");
    }

    inline static void IncrementCounter()
    {
        typedef Catcher<int> CountCatcher;

        Theron::Framework framework;
        Counter actor(framework);

        Theron::Receiver receiver;
        CountCatcher catcher;
        receiver.RegisterHandler(&catcher, &CountCatcher::Catch);

        framework.Send(1, receiver.GetAddress(), actor.GetAddress());
        framework.Send(2, receiver.GetAddress(), actor.GetAddress());
        framework.Send(3, receiver.GetAddress(), actor.GetAddress());
        framework.Send(4, receiver.GetAddress(), actor.GetAddress());
        framework.Send(5, receiver.GetAddress(), actor.GetAddress());
        framework.Send(6, receiver.GetAddress(), actor.GetAddress());

        framework.Send(true, receiver.GetAddress(), actor.GetAddress());

        receiver.Wait();

        Check(catcher.mMessage == 21, "Count is wrong");
    }

    inline static void ActorTemplate()
    {
        typedef Replier<int> IntReplier;

        Theron::Framework framework;
        IntReplier actor(framework);

        Theron::Receiver receiver;
        framework.Send(10, receiver.GetAddress(), actor.GetAddress());

        receiver.Wait();
    }

    inline static void OneHandlerAtATime()
    {
        typedef Catcher<int> CountCatcher;

        Theron::Framework framework;
        TwoHandlerCounter actor(framework);

        Theron::Receiver receiver;
        CountCatcher catcher;
        receiver.RegisterHandler(&catcher, &CountCatcher::Catch);

        framework.Send(2, receiver.GetAddress(), actor.GetAddress());
        framework.Send(0.0f, receiver.GetAddress(), actor.GetAddress());
        framework.Send(2, receiver.GetAddress(), actor.GetAddress());
        framework.Send(0.0f, receiver.GetAddress(), actor.GetAddress());
        framework.Send(2, receiver.GetAddress(), actor.GetAddress());
        framework.Send(0.0f, receiver.GetAddress(), actor.GetAddress());

        // Get the counter value.
        framework.Send(true, receiver.GetAddress(), actor.GetAddress());

        receiver.Wait();

        Check(catcher.mMessage == 9, "Count is wrong");
    }

    inline static void MultipleHandlersForMessageType()
    {
        typedef Catcher<int> CountCatcher;

        Theron::Framework framework;
        MultipleHandlerCounter actor(framework);

        Theron::Receiver receiver;
        CountCatcher catcher;
        receiver.RegisterHandler(&catcher, &CountCatcher::Catch);

        framework.Send(2, receiver.GetAddress(), actor.GetAddress());
        framework.Send(2, receiver.GetAddress(), actor.GetAddress());
        framework.Send(2, receiver.GetAddress(), actor.GetAddress());

        // Get the counter value.
        framework.Send(true, receiver.GetAddress(), actor.GetAddress());

        receiver.Wait();

        Check(catcher.mMessage == 9, "Count is wrong");
    }

    inline static void MessageArrivalOrder()
    {
        typedef Catcher<const char *> StringCatcher;
        typedef Sequencer<int> IntSequencer;

        Theron::Framework framework;
        IntSequencer actor(framework);

        Theron::Receiver receiver;
        StringCatcher catcher;
        receiver.RegisterHandler(&catcher, &StringCatcher::Catch);

        framework.Send(0, receiver.GetAddress(), actor.GetAddress());
        framework.Send(1, receiver.GetAddress(), actor.GetAddress());
        framework.Send(2, receiver.GetAddress(), actor.GetAddress());
        framework.Send(3, receiver.GetAddress(), actor.GetAddress());
        framework.Send(4, receiver.GetAddress(), actor.GetAddress());
        framework.Send(5, receiver.GetAddress(), actor.GetAddress());
        framework.Send(6, receiver.GetAddress(), actor.GetAddress());
        framework.Send(7, receiver.GetAddress(), actor.GetAddress());

        // Get the validity value.
        framework.Send(true, receiver.GetAddress(), actor.GetAddress());

        receiver.Wait();
        Check(catcher.mMessage == IntSequencer::GOOD, "Sequencer status is wrong");

        framework.Send(9, receiver.GetAddress(), actor.GetAddress());

        // Get the validity value.
        framework.Send(true, receiver.GetAddress(), actor.GetAddress());

        receiver.Wait();
        Check(catcher.mMessage == IntSequencer::BAD, "Sequencer status is wrong");
    }

    inline static void SendAddressAsMessage()
    {
        typedef Catcher<Theron::Address> AddressCatcher;

        Theron::Framework framework;
        Signaller actorA(framework);
        Signaller actorB(framework);

        Theron::Receiver receiver;
        AddressCatcher catcher;
        receiver.RegisterHandler(&catcher, &AddressCatcher::Catch);

        // Send A a message telling it to signal B.
        // A sends the receiver address to B as the signal,
        // causing B to send A's address to the receiver in turn.
        framework.Send(actorB.GetAddress(), receiver.GetAddress(), actorA.GetAddress());

        receiver.Wait();

        Check(catcher.mMessage == actorA.GetAddress(), "Wrong address");
    }

    inline static void SendMessageToDefaultHandlerInFunction()
    {
        typedef DefaultReplier<float> FloatReplier;
        typedef Catcher<std::string> StringCatcher;

        Theron::Framework framework;
        FloatReplier actor(framework);

        StringCatcher catcher;
        Theron::Receiver receiver;
        receiver.RegisterHandler(&catcher, &StringCatcher::Catch);

        // Send an int to the replier, which expects floats but has a default handler.
        framework.Send(52, receiver.GetAddress(), actor.GetAddress());

        receiver.Wait();

        Check(catcher.mMessage == "hello", "Default handler not executed");
    }

    inline static void RegisterHandlerFromHandler()
    {
        typedef Catcher<std::string> StringCatcher;

        Theron::Framework framework;
        Switcher actor(framework);

        StringCatcher catcher;
        Theron::Receiver receiver;
        receiver.RegisterHandler(&catcher, &StringCatcher::Catch);

        framework.Send(std::string("hello"), receiver.GetAddress(), actor.GetAddress());
        receiver.Wait();
        Check(catcher.mMessage == "hello", "Handler not executed");

        framework.Send(std::string("hello"), receiver.GetAddress(), actor.GetAddress());
        receiver.Wait();
        Check(catcher.mMessage == "goodbye", "Handler not executed");

        framework.Send(std::string("hello"), receiver.GetAddress(), actor.GetAddress());
        receiver.Wait();
        Check(catcher.mMessage == "hello", "Handler not executed");

        framework.Send(std::string("hello"), receiver.GetAddress(), actor.GetAddress());
        receiver.Wait();
        Check(catcher.mMessage == "goodbye", "Handler not executed");
    }

    inline static void CreateActorInConstructor()
    {
        Theron::Framework framework;

        Recursor::Parameters params;
        params.mCount = 10;
        Recursor actor(framework, params);
    }

    inline static void SendMessageInConstructor()
    {
        Theron::Framework framework;
        Theron::Receiver receiver;

        // Pass the address of the receiver to the actor constructor.
        AutoSender::Parameters params(receiver.GetAddress());
        AutoSender actor(framework, params);

        // Wait for the messages sent by the actor on construction.
        receiver.Wait();
        receiver.Wait();
    }

    inline static void DeregisterHandlerInConstructor()
    {
        typedef Catcher<int> IntCatcher;

        Theron::Framework framework;

        Theron::Receiver receiver;
        IntCatcher catcher;
        receiver.RegisterHandler(&catcher, &IntCatcher::Catch);

        AutoDeregistrar actor(framework);

        // Send the actor a message and check that the first handler doesn't send us a reply.
        framework.Send(0, receiver.GetAddress(), actor.GetAddress());

        receiver.Wait();
        Check(catcher.mMessage == 2, "Received wrong message");
        Check(receiver.Count() == 0, "Received too many messages");
    }

    inline static void CreateActorInDestructor()
    {
        Theron::Framework framework;

        TailRecursor::Parameters params;
        params.mCount = 10;
        TailRecursor actor(framework, params);
    }

    inline static void SendMessageInDestructor()
    {
        Theron::Framework framework;
        Theron::Receiver receiver;

        {
            // Pass the address of the receiver ton the actor constructor.
            TailSender::Parameters params(receiver.GetAddress());
            TailSender actor(framework, params);
        }

        // Wait for the messages sent by the actor on destruction.
        receiver.Wait();
        receiver.Wait();
    }

    inline static void DeregisterHandlerInDestructor()
    {
        // We check that it's safe to deregister a handler in an actor destructor,
        // but since it can't handle messages after destruction, there's little effect.
        Theron::Framework framework;
        TailDeregistrar actor(framework);
    }

    inline static void CreateActorInHandler()
    {
        Theron::Framework framework;
        Theron::Receiver receiver;
        Nestor actor(framework);

        framework.Send(Nestor::CreateMessage(), receiver.GetAddress(), actor.GetAddress());
        framework.Send(Nestor::DestroyMessage(), receiver.GetAddress(), actor.GetAddress());

        receiver.Wait();
    }

    inline static void GetNumQueuedMessagesInHandler()
    {
        typedef Catcher<Theron::uint32_t> CountCatcher;

        Theron::Framework framework;
        Theron::Receiver receiver;

        CountCatcher catcher;
        receiver.RegisterHandler(&catcher, &CountCatcher::Catch);
        
        MessageQueueCounter actor(framework);

        // Send the actor two messages.
        framework.Send(0, receiver.GetAddress(), actor.GetAddress());
        framework.Send(0, receiver.GetAddress(), actor.GetAddress());

        // Wait for and check both replies.
        // Race condition decides whether the second message has already arrived.
        // In Theron 4 the count includes the message currently being processed.
        receiver.Wait();
        Check(catcher.mMessage == 1 || catcher.mMessage == 2, "GetNumQueuedMessages failed");

        receiver.Wait();
        Check(catcher.mMessage == 1, "GetNumQueuedMessages failed");
    }

    inline static void GetNumQueuedMessagesInFunction()
    {
        typedef const char * StringMessage;
        typedef Catcher<Theron::uint32_t> CountCatcher;

        Theron::Framework framework;
        Theron::Receiver receiver;

        StringReplier actor(framework);

        // Send the actor two messages.
        StringMessage stringMessage("hello");
        framework.Send(stringMessage, receiver.GetAddress(), actor.GetAddress());
        framework.Send(stringMessage, receiver.GetAddress(), actor.GetAddress());

        // Race conditions decide how many messages are in the queue when we ask.
        // In Theron 4 the count includes the message currently being processed.
        Theron::uint32_t numMessages(actor.GetNumQueuedMessages());
        Check(numMessages < 3, "GetNumQueuedMessages failed, expected less than 3 messages");

        receiver.Wait();

        // There's no guarantee that the message handler will finish before the Wait() call returns.
        numMessages = actor.GetNumQueuedMessages();
        Check(numMessages < 3, "GetNumQueuedMessages failed, expected less than 3 messages");

        receiver.Wait();

        numMessages = actor.GetNumQueuedMessages();
        Check(numMessages < 2, "GetNumQueuedMessages failed, expected less than 2 messages");
    }

    inline static void UseBlindDefaultHandler()
    {
        typedef Accumulator<Theron::uint32_t> UIntAccumulator;

        Theron::Framework framework;
        Theron::Receiver receiver;

        UIntAccumulator accumulator;
        receiver.RegisterHandler(&accumulator, &UIntAccumulator::Catch);
        
        BlindActor actor(framework);

        // Send the actor a uint32_t message, which is the type it secretly expects.
        framework.Send(Theron::uint32_t(75), receiver.GetAddress(), actor.GetAddress());

        // The actor sends back the value of the message data and the size.
        receiver.Wait();
        receiver.Wait();

        Check(accumulator.Pop() == 75, "Bad blind data");
        Check(accumulator.Pop() == 4, "Bad blind data size");
    }

    inline static void IsHandlerRegisteredInHandler()
    {
        typedef Accumulator<bool> BoolAccumulator;

        Theron::Framework framework;
        Theron::Receiver receiver;

        BoolAccumulator accumulator;
        receiver.RegisterHandler(&accumulator, &BoolAccumulator::Catch);
        
        HandlerChecker actor(framework);
        framework.Send(int(0), receiver.GetAddress(), actor.GetAddress());

        Theron::uint32_t count(21);
        while (count)
        {
            count -= receiver.Wait(count);
        }

        Check(accumulator.Pop() == true, "Bad registration check result 0");
        Check(accumulator.Pop() == false, "Bad registration check result 1");
        Check(accumulator.Pop() == false, "Bad registration check result 2");

        //RegisterHandler(this, &HandlerChecker::Dummy);
        Check(accumulator.Pop() == true, "Bad registration check result 3");
        Check(accumulator.Pop() == true, "Bad registration check result 4");
        Check(accumulator.Pop() == false, "Bad registration check result 5");

        // DeregisterHandler(this, &HandlerChecker::Dummy);
        Check(accumulator.Pop() == true, "Bad registration check result 6");
        Check(accumulator.Pop() == false, "Bad registration check result 7");
        Check(accumulator.Pop() == false, "Bad registration check result 8");

        // DeregisterHandler(this, &HandlerChecker::Check);
        Check(accumulator.Pop() == false, "Bad registration check result 9");
        Check(accumulator.Pop() == false, "Bad registration check result 10");
        Check(accumulator.Pop() == false, "Bad registration check result 11");

        // RegisterHandler(this, &HandlerChecker::Dummy);
        // RegisterHandler(this, &HandlerChecker::Check);
        // RegisterHandler(this, &HandlerChecker::Check);
        Check(accumulator.Pop() == true, "Bad registration check result 12");
        Check(accumulator.Pop() == true, "Bad registration check result 13");
        Check(accumulator.Pop() == false, "Bad registration check result 14");

        // DeregisterHandler(this, &HandlerChecker::Check);
        Check(accumulator.Pop() == true, "Bad registration check result 15");
        Check(accumulator.Pop() == true, "Bad registration check result 16");
        Check(accumulator.Pop() == false, "Bad registration check result 17");

        // DeregisterHandler(this, &HandlerChecker::Check);
        Check(accumulator.Pop() == false, "Bad registration check result 18");
        Check(accumulator.Pop() == true, "Bad registration check result 19");
        Check(accumulator.Pop() == false, "Bad registration check result 20");
    }

    inline static void SetFallbackHandler()
    {
        Theron::Framework framework;
        FallbackHandler fallbackHandler;

        Check(framework.SetFallbackHandler(&fallbackHandler, &FallbackHandler::Handle), "Register failed");
    }

    inline static void HandleUndeliveredMessageSentInFunction()
    {
        typedef Replier<float> FloatReplier;

        Theron::Framework framework;
        Theron::Receiver receiver;

        FallbackHandler fallbackHandler;
        framework.SetFallbackHandler(&fallbackHandler, &FallbackHandler::Handle);

        // Create an actor and let it die but remember its address.
        Theron::Address staleAddress;

        {
            FloatReplier actor(framework);
            staleAddress = actor.GetAddress();
        }

        // Send a message to the stale address.
        framework.Send(0, receiver.GetAddress(), staleAddress);

        // Wait for the undelivered message to be caught by the registered fallback handler.
        uint32_t backoff(0);
        while (fallbackHandler.mAddress != receiver.GetAddress())
        {
            Theron::Detail::Utils::Backoff(backoff);
        }
    }

    inline static void HandleUnhandledMessageSentInFunction()
    {
        typedef Replier<Theron::uint32_t> UIntReplier;

        Theron::Framework framework;
        Theron::Receiver receiver;

        FallbackHandler fallbackHandler;
        framework.SetFallbackHandler(&fallbackHandler, &FallbackHandler::Handle);

        // Create a replier that handles only ints, then send it a float.
        UIntReplier replier(framework);
        framework.Send(5.0f, receiver.GetAddress(), replier.GetAddress());

        // Wait for the unhandled message to be caught by the registered fallback handler.
        uint32_t backoff(0);
        while (fallbackHandler.mAddress != receiver.GetAddress())
        {
            Theron::Detail::Utils::Backoff(backoff);
        }
    }

    inline static void HandleUndeliveredBlindMessageSentInFunction()
    {
        typedef Replier<float> FloatReplier;

        Theron::Framework framework;
        Theron::Receiver receiver;

        BlindFallbackHandler fallbackHandler;
        fallbackHandler.mData = 0;
        framework.SetFallbackHandler(&fallbackHandler, &BlindFallbackHandler::Handle);

        // Create an actor and let it die but remember its address.
        Theron::Address staleAddress;

        {
            FloatReplier actor(framework);
            staleAddress = actor.GetAddress();
        }

        // Send a message to the stale address.
        framework.Send(Theron::uint32_t(42), receiver.GetAddress(), staleAddress);

        // Wait for the undelivered message to be caught by the registered fallback handler.
        uint32_t backoff(0);
        while (fallbackHandler.mData == 0)
        {
            Theron::Detail::Utils::Backoff(backoff);
        }

        // Check that the undelivered message was handled by the registered fallback handler.
        Check(fallbackHandler.mValue == 42, "Blind fallback handler collected bad value");
        Check(fallbackHandler.mSize == sizeof(Theron::uint32_t), "Blind fallback handler collected bad size");
        Check(fallbackHandler.mAddress == receiver.GetAddress(), "Blind fallback handler collected bad address");
    }

    inline static void HandleMessageSentToStaleFrameworkInFunction()
    {
        typedef Replier<float> FloatReplier;

        Theron::Framework framework0;
        Theron::Receiver receiver;

        // Register a safe fallback handler in the first framework.
        FallbackHandler fallbackHandler;
        framework0.SetFallbackHandler(&fallbackHandler, &FallbackHandler::Handle);

        // Create an actor in the first framework.
        FloatReplier replier0(framework0);

        // Create an actor in a second framework, then let the framework die but remember the address.
        Theron::Address staleAddress;

        {
            Theron::Framework framework1;
            FloatReplier replier1(framework1);
            staleAddress = replier1.GetAddress();
        }

        // Send a message from the first framework to the stale address in the second framework.
        framework0.Send(0.0f, receiver.GetAddress(), staleAddress);

        // Wait for the undelivered message to be caught by the registered fallback handler.
        uint32_t backoff(0);
        while (fallbackHandler.mAddress != receiver.GetAddress())
        {
            Theron::Detail::Utils::Backoff(backoff);
        }
    }

    inline static void HandleMessageSentToStaleFrameworkInHandler()
    {
        typedef Replier<float> FloatReplier;

        Theron::Framework framework0;
        Theron::Receiver receiver;

        // Register a safe fallback handler in the first framework.
        FallbackHandler fallbackHandler;
        framework0.SetFallbackHandler(&fallbackHandler, &FallbackHandler::Handle);

        // Create an actor in the first framework to send a message to an actor in the second.
        // The signaller sends a message to the address it is sent.
        Signaller signaller(framework0);

        // Create an actor in a second framework, then let the framework die but remember the address.
        Theron::Address staleAddress;

        {
            Theron::Framework framework1;
            FloatReplier replier(framework1);
            staleAddress = replier.GetAddress();
        }

        // Send a message to the signaller telling it to send a message to the stale address.
        framework0.Send(staleAddress, receiver.GetAddress(), signaller.GetAddress());

        // Wait for the undelivered message to be caught by the registered fallback handler.
        // The undelivered message was sent by the signaller.
        uint32_t backoff(0);
        while (fallbackHandler.mAddress != signaller.GetAddress())
        {
            Theron::Detail::Utils::Backoff(backoff);
        }
    }

    inline static void SendRegisteredMessage()
    {
        typedef Replier<IntVectorMessage> IntVectorReplier;
        typedef Catcher<IntVectorMessage> IntVectorCatcher;

        Theron::Framework framework;

        Theron::Receiver receiver;
        IntVectorCatcher catcher;
        receiver.RegisterHandler(&catcher, &IntVectorCatcher::Catch);

        IntVectorReplier replier(framework);

        IntVectorMessage message;
        message.push_back(0);
        message.push_back(1);
        message.push_back(2);

        framework.Send(message, receiver.GetAddress(), replier.GetAddress());
        receiver.Wait();

        Check(catcher.mMessage.size() == 3, "Bad reply message");
        Check(catcher.mMessage[0] == 0, "Bad reply message");
        Check(catcher.mMessage[1] == 1, "Bad reply message");
        Check(catcher.mMessage[2] == 2, "Bad reply message");
    }

    inline static void DeriveFromActorFirst()
    {
        typedef Catcher<int> IntCatcher;

        Theron::Framework framework;

        Theron::Receiver receiver;
        IntCatcher catcher;
        receiver.RegisterHandler(&catcher, &IntCatcher::Catch);

        ActorFirst actor(framework);

        framework.Send(5, receiver.GetAddress(), actor.GetAddress());
        receiver.Wait();

        Check(catcher.mMessage == 5, "Bad reply message");
    }

    inline static void DeriveFromActorLast()
    {
        typedef Catcher<int> IntCatcher;

        Theron::Framework framework;

        Theron::Receiver receiver;
        IntCatcher catcher;
        receiver.RegisterHandler(&catcher, &IntCatcher::Catch);

        ActorLast actor(framework);

        framework.Send(5, receiver.GetAddress(), actor.GetAddress());
        receiver.Wait();

        Check(catcher.mMessage == 5, "Bad reply message");
    }

    inline static void SendEmptyMessage()
    {
        typedef Replier<EmptyMessage> EmptyReplier;
        typedef Catcher<EmptyMessage> EmptyCatcher;

        Theron::Framework framework;

        Theron::Receiver receiver;
        EmptyCatcher catcher;
        receiver.RegisterHandler(&catcher, &EmptyCatcher::Catch);

        EmptyReplier replier(framework);

        framework.Send(EmptyMessage(), receiver.GetAddress(), replier.GetAddress());
        receiver.Wait();

        Check(&catcher.mMessage != 0, "No reply message");
    }

    inline static void MultipleFrameworks()
    {
        typedef Catcher<int> IntCatcher;

        Theron::Receiver receiver;
        IntCatcher catcher;
        receiver.RegisterHandler(&catcher, &IntCatcher::Catch);

        Theron::Framework framework0;
        Theron::Framework framework1;
        Theron::Framework framework2;
        Theron::Framework framework3;
        Theron::Framework framework4;

        Forwarder actor0(framework0, receiver.GetAddress());
        Forwarder actor1(framework1, actor0.GetAddress());
        Forwarder actor2(framework2, actor1.GetAddress());
        Forwarder actor3(framework3, actor2.GetAddress());
        Forwarder actor4(framework4, actor3.GetAddress());

        framework0.Send(int(5), receiver.GetAddress(), actor4.GetAddress());

        receiver.Wait();
        Check(catcher.mMessage == 0, "Received wrong message");
        Check(receiver.Count() == 0, "Received too many messages");
    }

    inline static void ConstructFrameworkWithParameters()
    {
        typedef Replier<int> IntReplier;

        const Theron::Framework::Parameters params0;
        Theron::Framework framework0(params0);
        Theron::Framework framework1(Theron::Framework::Parameters(8));
        Theron::Framework framework2(Theron::Framework::Parameters(12, 0));

        IntReplier replier0(framework0);
        IntReplier replier1(framework1);
        IntReplier replier2(framework2);

        Theron::Receiver receiver;
        framework0.Send(int(2), receiver.GetAddress(), replier0.GetAddress());
        framework1.Send(int(2), receiver.GetAddress(), replier1.GetAddress());
        framework2.Send(int(2), receiver.GetAddress(), replier2.GetAddress());

        receiver.Wait();
        receiver.Wait();
        receiver.Wait();
    }

    inline static void ThreadCountApi()
    {
        Theron::Framework framework;

        Theron::Detail::Utils::SleepThread(10);

        // Create more worker threads.
        framework.SetMinThreads(32);
        Check(framework.GetMinThreads() >= 32, "GetMinThreads failed");

        Theron::Detail::Utils::SleepThread(10);

        // Stop most of the threads.
        framework.SetMaxThreads(8);
        Check(framework.GetMaxThreads() <= 8, "GetMaxThreads failed");

        Theron::Detail::Utils::SleepThread(10);

        // Re-start all of the threads and create some more.
        framework.SetMinThreads(64);
        Check(framework.GetMinThreads() >= 64, "GetMinThreads failed");

        Theron::Detail::Utils::SleepThread(10);

        // Stop all threads but one.
        framework.SetMaxThreads(1);
        Check(framework.GetMaxThreads() <= 1, "GetMaxThreads failed");

        Theron::Detail::Utils::SleepThread(10);

        Check(framework.GetPeakThreads() >= 1, "GetPeakThreads failed");
        Check(framework.GetNumThreads() >= 1, "GetNumThreads failed");
        Check(framework.GetPeakThreads() >= framework.GetNumThreads(), "GetPeakThreads failed");
    }

    inline static void EventCounterApi()
    {
#if THERON_ENABLE_COUNTERS
        typedef Replier<int> IntReplier;

        Theron::Framework framework;
        Theron::Receiver receiver;

        Theron::Detail::Utils::SleepThread(10);

        // Check initial values.
        Check(framework.GetCounterValue(0) == 0, "GetCounterValue failed");

        uint32_t counterValues[32];
        uint32_t valueCount(framework.GetPerThreadCounterValues(0, counterValues, 32));

        uint32_t messagesProcessed(0);
        for (uint32_t index = 0; index < valueCount; ++index)
        {
            messagesProcessed += counterValues[index];
        }

        Check(messagesProcessed == 0, "GetPerThreadCounterValues failed");

        IntReplier replier(framework);
        framework.Send(int(0), receiver.GetAddress(), replier.GetAddress());
        receiver.Wait();

        // Check values after some work.
        Check(framework.GetCounterValue(0) > 0, "GetCounterValue failed");

        valueCount = framework.GetPerThreadCounterValues(0, counterValues, 32);

        messagesProcessed = 0;
        for (uint32_t index = 0; index < valueCount; ++index)
        {
            messagesProcessed += counterValues[index];
        }

        Check(messagesProcessed == 1, "GetPerThreadCounterValues failed");

        // Check values after reset.
        framework.ResetCounters();

        Check(framework.GetCounterValue(0) == 0, "GetCounterValue failed");

        valueCount = framework.GetPerThreadCounterValues(0, counterValues, 32);

        messagesProcessed = 0;
        for (uint32_t index = 0; index < valueCount; ++index)
        {
            messagesProcessed += counterValues[index];
        }

        Check(messagesProcessed == 0, "GetPerThreadCounterValues failed");
#endif
    }

    inline static void ConstructEndPoint()
    {
        // Should be able to use endpoints even if networking is disabled.
        Theron::EndPoint endPoint("endpoint", "inproc://endpoint");
    }

    inline static void TieFrameworkToEndPoint()
    {
        Theron::EndPoint endPoint("endpoint", "inproc://endpoint");
        Theron::Framework framework(endPoint);
    }

    inline static void TieActorsToEndPoint()
    {
        typedef Replier<int> IntReplier;

        Theron::EndPoint endPoint("endpoint", "inproc://endpoint");
        Theron::Framework framework(endPoint);

        // Create two actors and let them register and deregister themselves with the endPoint.
        {
            IntReplier replierOne(framework);
            IntReplier replierTwo(framework);
        }

        // And again.
        {
            IntReplier replierOne(framework);
            IntReplier replierTwo(framework);
        }
    }

    inline static void TieReceiverstoEndPoint()
    {
        Theron::EndPoint endPoint("endpoint", "inproc://endpoint");

        // Create two receivers and let them register and deregister themselves with the endPoint.
        {
            Theron::Receiver receiverOne(endPoint);
            Theron::Receiver receiverTwo(endPoint);
        }

        // And again.
        {
            Theron::Receiver receiverOne(endPoint);
            Theron::Receiver receiverTwo(endPoint);
        }
    }

    inline static void NameFrameworkOnConstruction()
    {
        typedef Replier<int> IntReplier;

        Theron::EndPoint endPoint("endpoint", "inproc://endpoint");

        Theron::Framework::Parameters frameworkParams;
        Theron::Framework framework(endPoint, "framework", frameworkParams);

        // Register an actor and a receiver to test the name can be used.
        IntReplier replier(framework);
        Theron::Receiver receiver(endPoint);

        framework.Send(int(0), receiver.GetAddress(), replier.GetAddress());
        receiver.Wait();
    }

    inline static void NameActorOnConstruction()
    {
        typedef Replier<int> IntReplier;

        Theron::EndPoint endPoint("endpoint", "inproc://endpoint");
        Theron::Framework framework(endPoint);

        // Register a named actor and a receiver to test the name can be used.
        IntReplier replier(framework, "replier");
        Theron::Receiver receiver(endPoint);

        framework.Send(int(0), receiver.GetAddress(), Theron::Address("replier"));
        receiver.Wait();
    }

    inline static void NameReceiverOnConstruction()
    {
        typedef Replier<int> IntReplier;

        Theron::EndPoint endPoint("endpoint", "inproc://endpoint");
        Theron::Framework framework(endPoint);

        // Register a named actor and a named receiver.
        IntReplier replier(framework, "replier");
        Theron::Receiver receiver(endPoint, "receiver");

        framework.Send(int(0), Theron::Address("receiver"), Theron::Address("replier"));
        receiver.Wait();
    }

    inline static void SendMessageToLocalActorByName()
    {
        typedef Replier<std::string> StringReplier;

        Theron::EndPoint endPoint("endpoint", "inproc://endpoint");
        Theron::Framework framework(endPoint);

        StringReplier replier(framework, "replier");
        Theron::Receiver receiver(endPoint, "receiver");

        // This tests we can send any copyable message to actors on the same endpoint
        // by name, even if the message type hasn't been registered.
        const std::string message("hello world");

        // Send the replier a message by name, and pass the receiver address by name.
        framework.Send(message, Theron::Address("receiver"), Theron::Address("replier"));

        // Wait for the reply.
        receiver.Wait();
    }

    inline static void SendMessagesBetweenLocalFrameworksByName()
    {
        Theron::EndPoint endPoint("endpoint", "inproc://endpoint");
        Theron::Framework frameworkOne(endPoint);
        Theron::Framework frameworkTwo(endPoint);

        Signaller signallerOne(frameworkOne, "signaller_one");
        Signaller signallerTwo(frameworkTwo, "signaller_two");
        Theron::Receiver receiver(endPoint, "receiver");

        // Send one a message telling it to signal two.
        // One sends the receiver address to two as the signal,
        // causing two to send one's address to the receiver.
        Check(frameworkTwo.Send(
            Theron::Address("signaller_two"),
            Theron::Address("receiver"),
            Theron::Address("signaller_one")), "Failed to send message");

        receiver.Wait();
    }

private:

    class TrivialActor : public Theron::Actor
    {
    public:

        TrivialActor(Theron::Framework &framework) : Theron::Actor(framework)
        {
        }
    };

    template <class MessageType>
    class Registrar : public Theron::Actor
    {
    public:

        Registrar(Theron::Framework &framework) : Theron::Actor(framework)
        {
            RegisterHandler(this, &Registrar::Handler);
            IsHandlerRegistered(this, &Registrar::Handler);
            DeregisterHandler(this, &Registrar::Handler);
            IsHandlerRegistered(this, &Registrar::Handler);
        }

    private:

        inline void Handler(const MessageType &/*message*/, const Theron::Address /*from*/)
        {
        }
    };

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
    class DefaultReplier : public Theron::Actor
    {
    public:

        inline DefaultReplier(Theron::Framework &framework) : Theron::Actor(framework)
        {
            RegisterHandler(this, &DefaultReplier::Handler);
            SetDefaultHandler(this, &DefaultReplier::DefaultHandler);
        }

    private:

        inline void Handler(const MessageType &message, const Theron::Address from)
        {
            Send(message, from);
        }

        inline void DefaultHandler(const Theron::Address from)
        {
            std::string hello("hello");
            Send(hello, from);
        }
    };

    class StringReplier : public Replier<const char *>
    {
    public:

        typedef Replier<const char *> Base;

        inline StringReplier(Theron::Framework &framework) : Base(framework)
        {
        }
    };

    class Signaller : public Theron::Actor
    {
    public:

        inline Signaller(Theron::Framework &framework, const char *const name = 0) : Theron::Actor(framework, name)
        {
            RegisterHandler(this, &Signaller::Signal);
        }

    private:

        inline void Signal(const Theron::Address &address, const Theron::Address from)
        {
            // Send the 'from' address to the address received in the message.
            Send(from, address);
        }
    };

    class Switcher : public Theron::Actor
    {
    public:

        inline Switcher(Theron::Framework &framework) : Theron::Actor(framework)
        {
            RegisterHandler(this, &Switcher::SayHello);
        }

    private:

        inline void SayHello(const std::string &/*message*/, const Theron::Address from)
        {
            DeregisterHandler(this, &Switcher::SayHello);
            RegisterHandler(this, &Switcher::SayGoodbye);
            Send(std::string("hello"), from);
        }

        inline void SayGoodbye(const std::string &/*message*/, const Theron::Address from)
        {
            DeregisterHandler(this, &Switcher::SayGoodbye);
            RegisterHandler(this, &Switcher::SayHello);
            Send(std::string("goodbye"), from);
        }
    };

    template <class MessageType>
    class Catcher
    {
    public:

        inline Catcher() : mMessage(), mFrom(Theron::Address::Null())
        {
        }

        inline void Catch(const MessageType &message, const Theron::Address from)
        {
            mMessage = message;
            mFrom = from;
        }

        MessageType mMessage;
        Theron::Address mFrom;
    };

    class Counter : public Theron::Actor
    {
    public:

        inline Counter(Theron::Framework &framework) : Theron::Actor(framework), mCount(0)
        {
            RegisterHandler(this, &Counter::Increment);
            RegisterHandler(this, &Counter::GetValue);
        }

    private:

        inline void Increment(const int &message, const Theron::Address /*from*/)
        {
            mCount += message;
        }

        inline void GetValue(const bool &/*message*/, const Theron::Address from)
        {
            Send(mCount, from);
        }

        int mCount;
    };

    class TwoHandlerCounter : public Theron::Actor
    {
    public:

        inline TwoHandlerCounter(Theron::Framework &framework) : Theron::Actor(framework), mCount(0)
        {
            RegisterHandler(this, &TwoHandlerCounter::IncrementOne);
            RegisterHandler(this, &TwoHandlerCounter::IncrementTwo);
            RegisterHandler(this, &TwoHandlerCounter::GetValue);
        }

    private:

        inline void IncrementOne(const int &message, const Theron::Address /*from*/)
        {
            mCount += message;
        }

        inline void IncrementTwo(const float &/*message*/, const Theron::Address /*from*/)
        {
            ++mCount;
        }

        inline void GetValue(const bool &/*message*/, const Theron::Address from)
        {
            Send(mCount, from);
        }

        int mCount;
    };

    class MultipleHandlerCounter : public Theron::Actor
    {
    public:

        inline MultipleHandlerCounter(Theron::Framework &framework) : Theron::Actor(framework), mCount(0)
        {
            RegisterHandler(this, &MultipleHandlerCounter::IncrementOne);
            RegisterHandler(this, &MultipleHandlerCounter::IncrementTwo);
            RegisterHandler(this, &MultipleHandlerCounter::GetValue);
        }

    private:

        inline void IncrementOne(const int &message, const Theron::Address /*from*/)
        {
            mCount += message;
        }

        inline void IncrementTwo(const int &/*message*/, const Theron::Address /*from*/)
        {
            ++mCount;
        }

        inline void GetValue(const bool &/*message*/, const Theron::Address from)
        {
            Send(mCount, from);
        }

        int mCount;
    };

    template <class CountType>
    class Sequencer : public Theron::Actor
    {
    public:

        static const char *GOOD;
        static const char *BAD;

        inline Sequencer(Theron::Framework &framework) : Theron::Actor(framework), mNextValue(0), mStatus(GOOD)
        {
            RegisterHandler(this, &Sequencer::Receive);
            RegisterHandler(this, &Sequencer::GetValue);
        }

    private:

        inline void Receive(const CountType &message, const Theron::Address /*from*/)
        {
            if (message != mNextValue++)
            {
                mStatus = BAD;
            }
        }

        inline void GetValue(const bool &/*message*/, const Theron::Address from)
        {
            Send(mStatus, from);
        }

        CountType mNextValue;
        const char *mStatus;
    };

    class Recursor : public Theron::Actor
    {
    public:

        struct Parameters
        {
            int mCount;
        };

        inline Recursor(Theron::Framework &framework, const Parameters &params) : Theron::Actor(framework)
        {
            // Recursively create a child actor within the constructor.
            if (params.mCount > 0)
            {
                Parameters childParams;
                childParams.mCount = params.mCount - 1;

                Theron::Framework &framework(GetFramework());
                Recursor child(framework, childParams);
            }
        }
    };

    class TailRecursor : public Theron::Actor
    {
    public:

        struct Parameters
        {
            int mCount;
        };

        inline TailRecursor(Theron::Framework &framework, const Parameters &params) : Theron::Actor(framework), mCount(params.mCount)
        {
        }

        inline ~TailRecursor()
        {
            // Recursively create a child actor within the destructor.
            if (mCount > 0)
            {
                Parameters childParams;
                childParams.mCount = mCount - 1;

                Theron::Framework &framework(GetFramework());
                TailRecursor child(framework, childParams);
            }
        }

    private:

        int mCount;
    };

    class AutoSender : public Theron::Actor
    {
    public:

        typedef Theron::Address Parameters;

        inline AutoSender(Theron::Framework &framework, const Parameters &address) : Theron::Actor(framework)
        {
            // Send a message in the actor constructor.
            Send(0, address);

            // Send using TailSend to check that works too.
            TailSend(1, address);
        }
    };

    class TailSender : public Theron::Actor
    {
    public:

        typedef Theron::Address Parameters;

        inline TailSender(Theron::Framework &framework, const Parameters &address) : Theron::Actor(framework), mStoredAddress(address)
        {
        }

        inline ~TailSender()
        {
            // Send a message in the actor destructor.
            Send(0, mStoredAddress);

            // Send using TailSend to check that works too.
            TailSend(1, mStoredAddress);
        }

        Theron::Address mStoredAddress;
    };

    class AutoDeregistrar : public Theron::Actor
    {
    public:

        inline AutoDeregistrar(Theron::Framework &framework) : Theron::Actor(framework)
        {
            RegisterHandler(this, &AutoDeregistrar::HandlerOne);
            RegisterHandler(this, &AutoDeregistrar::HandlerTwo);
            DeregisterHandler(this, &AutoDeregistrar::HandlerOne);
        }

        inline void HandlerOne(const int &/*message*/, const Theron::Address from)
        {
            Send(1, from);
        }

        inline void HandlerTwo(const int &/*message*/, const Theron::Address from)
        {
            Send(2, from);
        }
    };

    class TailDeregistrar : public Theron::Actor
    {
    public:

        inline TailDeregistrar(Theron::Framework &framework) : Theron::Actor(framework)
        {
            RegisterHandler(this, &TailDeregistrar::Handler);
        }

        inline ~TailDeregistrar()
        {
            DeregisterHandler(this, &TailDeregistrar::Handler);
        }

        inline void Handler(const int &/*message*/, const Theron::Address from)
        {
            Send(0, from);
        }
    };

    class MessageQueueCounter : public Theron::Actor
    {
    public:

        inline explicit MessageQueueCounter(Theron::Framework &framework) : Theron::Actor(framework)
        {
            RegisterHandler(this, &MessageQueueCounter::Handler);
        }

        inline void Handler(const int &/*message*/, const Theron::Address from)
        {
            Send(GetNumQueuedMessages(), from);
        }
    };

    class BlindActor : public Theron::Actor
    {
    public:

        inline BlindActor(Theron::Framework &framework) : Theron::Actor(framework)
        {
            SetDefaultHandler(this, &BlindActor::BlindDefaultHandler);
        }

    private:

        inline void BlindDefaultHandler(const void *const data, const Theron::uint32_t size, const Theron::Address from)
        {
            // We know the message is a uint32_t.
            const Theron::uint32_t *const p(reinterpret_cast<const Theron::uint32_t *>(data));
            const Theron::uint32_t value(*p);

            Send(value, from);
            Send(size, from);
        }
    };

    template <class MessageType>
    class Accumulator
    {
    public:

        inline Accumulator() : mMessages()
        {
        }

        inline void Catch(const MessageType &message, const Theron::Address /*from*/)
        {
            mMessages.push(message);
        }

        int Size()
        {
            return mMessages.size();
        }

        MessageType Pop()
        {
            THERON_ASSERT(mMessages.empty() == false);
            MessageType message(mMessages.front());
            mMessages.pop();
            return message;
        }

    private:

        std::queue<MessageType> mMessages;
    };

    class HandlerChecker : public Theron::Actor
    {
    public:

        inline HandlerChecker(Theron::Framework &framework) : Theron::Actor(framework)
        {
            RegisterHandler(this, &HandlerChecker::Check);
        }

    private:

        inline void Check(const int & /*message*/, const Theron::Address from)
        {
            Send(IsHandlerRegistered(this, &HandlerChecker::Check), from);
            Send(IsHandlerRegistered(this, &HandlerChecker::Dummy), from);
            Send(IsHandlerRegistered(this, &HandlerChecker::Unregistered), from);

            RegisterHandler(this, &HandlerChecker::Dummy);

            Send(IsHandlerRegistered(this, &HandlerChecker::Check), from);
            Send(IsHandlerRegistered(this, &HandlerChecker::Dummy), from);
            Send(IsHandlerRegistered(this, &HandlerChecker::Unregistered), from);

            DeregisterHandler(this, &HandlerChecker::Dummy);

            Send(IsHandlerRegistered(this, &HandlerChecker::Check), from);
            Send(IsHandlerRegistered(this, &HandlerChecker::Dummy), from);
            Send(IsHandlerRegistered(this, &HandlerChecker::Unregistered), from);

            DeregisterHandler(this, &HandlerChecker::Check);

            Send(IsHandlerRegistered(this, &HandlerChecker::Check), from);
            Send(IsHandlerRegistered(this, &HandlerChecker::Dummy), from);
            Send(IsHandlerRegistered(this, &HandlerChecker::Unregistered), from);

            RegisterHandler(this, &HandlerChecker::Dummy);
            RegisterHandler(this, &HandlerChecker::Check);
            RegisterHandler(this, &HandlerChecker::Check);

            Send(IsHandlerRegistered(this, &HandlerChecker::Check), from);
            Send(IsHandlerRegistered(this, &HandlerChecker::Dummy), from);
            Send(IsHandlerRegistered(this, &HandlerChecker::Unregistered), from);
        
            DeregisterHandler(this, &HandlerChecker::Check);

            Send(IsHandlerRegistered(this, &HandlerChecker::Check), from);
            Send(IsHandlerRegistered(this, &HandlerChecker::Dummy), from);
            Send(IsHandlerRegistered(this, &HandlerChecker::Unregistered), from);

            DeregisterHandler(this, &HandlerChecker::Check);

            Send(IsHandlerRegistered(this, &HandlerChecker::Check), from);
            Send(IsHandlerRegistered(this, &HandlerChecker::Dummy), from);
            Send(IsHandlerRegistered(this, &HandlerChecker::Unregistered), from);
        }

        inline void Dummy(const int & message, const Theron::Address from)
        {
            // We do this just so that Dummy and Unregistered differ, so the compiler won't merge them!
            Send(message, from);
        }

        inline void Unregistered(const int & /*message*/, const Theron::Address /*from*/)
        {
        }
    };

    class Nestor : public Theron::Actor
    {
    public:

        struct CreateMessage { };
        struct DestroyMessage { };

        inline Nestor(Theron::Framework &framework) : Theron::Actor(framework)
        {
            RegisterHandler(this, &Nestor::Create);
            RegisterHandler(this, &Nestor::Destroy);
            RegisterHandler(this, &Nestor::Receive);
        }

    private:

        typedef Replier<int> ChildActor;

        inline void Create(const CreateMessage & /*message*/, const Theron::Address /*from*/)
        {
            mChildren.push_back(new ChildActor(GetFramework()));
            mChildren.push_back(new ChildActor(GetFramework()));
            mChildren.push_back(new ChildActor(GetFramework()));

            mReplies.push_back(false);
            mReplies.push_back(false);
            mReplies.push_back(false);

            Send(0, mChildren[0]->GetAddress());
            Send(1, mChildren[1]->GetAddress());
            Send(2, mChildren[2]->GetAddress());
        }

        inline void Destroy(const DestroyMessage & /*message*/, const Theron::Address from)
        {
            mCaller = from;
            if (mReplies[0] && mReplies[1] && mReplies[2])
            {
                delete mChildren[0];
                delete mChildren[1];
                delete mChildren[2];

                mChildren.clear();
                Send(true, mCaller);
            }
        }

        inline void Receive(const int & message, const Theron::Address /*from*/)
        {
            mReplies[message] = true;

            if (mCaller != Theron::Address::Null() && mReplies[0] && mReplies[1] && mReplies[2])
            {
                delete mChildren[0];
                delete mChildren[1];
                delete mChildren[2];

                mChildren.clear();
                Send(true, mCaller);
            }
        }

        std::vector<Theron::Actor *> mChildren;
        std::vector<bool> mReplies;
        Theron::Address mCaller;
    };

    class FallbackHandler
    {
    public:

        inline void Handle(const Theron::Address from)
        {
            mAddress = from;
        }

        Theron::Address mAddress;
    };

    class BlindFallbackHandler
    {
    public:

        BlindFallbackHandler() : mData(0), mValue(0), mSize(0)
        {
        }

        inline void Handle(const void *const data, const Theron::uint32_t size, const Theron::Address from)
        {
            mData = data;
            mValue = *reinterpret_cast<const Theron::uint32_t *>(data);
            mSize = size;
            mAddress = from;
        }

        const void *mData;
        Theron::uint32_t mValue;
        Theron::uint32_t mSize;
        Theron::Address mAddress;
    };

    typedef std::vector<Theron::uint32_t> IntVectorMessage;

    class SomeOtherBaseclass
    {
    public:

        inline SomeOtherBaseclass()
        {
        }

        // The virtual destructor is required because this is a baseclass with virtual functions.
        inline virtual ~SomeOtherBaseclass()
        {
        }

        inline virtual void DoNothing()
        {
        }
    };

    class ActorFirst : public Theron::Actor, public SomeOtherBaseclass
    {
    public:

        inline ActorFirst(Theron::Framework &framework) : Theron::Actor(framework)
        {
            RegisterHandler(this, &ActorFirst::Handler);
        }

        // The virtual destructor is required because we derived from baseclasses with virtual functions.
        inline virtual ~ActorFirst()
        {
        }

    private:

        inline virtual void DoNothing()
        {
        }

        inline void Handler(const int &message, const Theron::Address from)
        {
            Send(message, from);
        }
    };

    class ActorLast : public SomeOtherBaseclass, public Theron::Actor
    {
    public:

        inline ActorLast(Theron::Framework &framework) : Theron::Actor(framework)
        {
            RegisterHandler(this, &ActorLast::Handler);
        }

        // The virtual destructor is required because we derived from baseclasses with virtual functions.
        inline virtual ~ActorLast()
        {
        }

    private:

        inline virtual void DoNothing()
        {
        }
        
        inline void Handler(const int &message, const Theron::Address from)
        {
            Send(message, from);
        }
    };

    struct EmptyMessage
    {
    };

    class Forwarder : public Theron::Actor
    {
    public:

        inline Forwarder(Theron::Framework &framework, const Theron::Address next) : Theron::Actor(framework), mNext(next)
        {
            RegisterHandler(this, &Forwarder::Forward);
        }

    private:

        inline void Forward(const int &message, const Theron::Address /*from*/)
        {
            Send(message - 1, mNext);
        }

        const Theron::Address mNext;
    };
};


template <class CountType>
const char *FeatureTestSuite::Sequencer<CountType>::GOOD = "good";

template <class CountType>
const char *FeatureTestSuite::Sequencer<CountType>::BAD = "good";


} // namespace Tests


THERON_REGISTER_MESSAGE(Tests::FeatureTestSuite::IntVectorMessage);


#endif // THERON_TESTS_TESTSUITES_FEATURETESTSUITE_H
