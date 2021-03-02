#pragma once

#include<iostream>
#include<deque>
#include<thread>
#include<mutex>
#include<condition_variable>
#include<random>
#include<chrono>

std::mutex m;
std::condition_variable cond;
constexpr int MAX_COUNTER_SIZE = 10;
static int numberOfcounters = 0;

enum class TicketType : int { A = 0, B = 1, C = 2 };

class Ticket
{
public:
	Ticket():duration(0), type(TicketType::A) {}
	Ticket(const int time, const TicketType t) : duration(time), type(t)
	{}
	int getDuration() const { return duration; }
	TicketType getTicketType() const { return type; }
private:
	int duration;
	TicketType type;
};

using TicketQueue = std::deque<Ticket>;

class TicketGenerator
{
public:
	TicketGenerator(TicketQueue& tsksA, TicketQueue& tsksB, TicketQueue& tsksC);
	void operator()();
private:

	TicketQueue& ticketsA;
	TicketQueue& ticketsB;
	TicketQueue& ticketsC;
};


class Counter
{
public:
	Counter(TicketQueue& tckts, const TicketType counterType);
	void operator()();
	void open();
	void close();
	Ticket& getCurrentTicket();

private:
	void getFirstTicketinQueue();
	bool m_closed;
	TicketQueue& tickets;
	int m_counterNumber;
	TicketType m_counterType;
	Ticket m_currentTicket;
};


