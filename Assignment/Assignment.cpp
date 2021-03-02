	#include "Assignment.h"

std::ostream& operator << (std::ostream& o, const TicketType& t)
{
	switch (t)
	{
	case TicketType::A:
		o << "A";
		break;
	case TicketType::B:
		o << "B";
		break;
	case TicketType::C:
		o << "C";
		break;
	}
	return o;
}

TicketGenerator::TicketGenerator(TicketQueue& tsksA, TicketQueue& tsksB, TicketQueue& tsksC)
	: ticketsA(tsksA), ticketsB(tsksB), ticketsC(tsksC){}



void TicketGenerator::operator()()
{
		int count = 0;
		//Task waiting time is between 20 to 60 seconds. 
		const int maxWaitingTime = 60;
		const int minWaitingTime = 20; 

		while (true)
		{
		
			// Create ticket with random ticket type (with serving time between 20-60 seconds)
			int durationBetweenTwoNumbers = rand() % (maxWaitingTime - minWaitingTime + 1) + minWaitingTime;
			Ticket ticket(durationBetweenTwoNumbers, TicketType(rand() % 3));

			std::unique_lock<std::mutex> lck(m);
			
			// Add the ticket in one of the following lists depending on their ticket type. 
			switch (ticket.getTicketType())
			{
				case TicketType::A :
					ticketsA.push_back(ticket);
					break;
				case TicketType::B :
					ticketsB.push_back(ticket);
					break;
				case TicketType::C:
					ticketsC.push_back(ticket);
					break;
			}
			
			std::cout << " Customer entered (T:" << ticket.getDuration() << ", Counter:" << ticket.getTicketType()  << ")"<<std::endl;
			lck.unlock();
			cond.notify_all();

			// ToDo: Calculate the accurate wait time depending on poisson's probability model in order to have maximum 3 tasks per minute.
			std::this_thread::sleep_for(std::chrono::seconds(20));
		}
}


Counter::Counter(TicketQueue& tckts, const TicketType counterType) : tickets(tckts), m_counterType(counterType), m_closed(false) {
		m_counterNumber = ++numberOfcounters;
}

void Counter::open()
{
	m_closed = false;
}
void Counter::close()
{
	m_closed = true;
}

void Counter::operator()()
{

		std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

		while (true)
		{
			// Check if the 5 mins elapsed? if yes then toggle m_closed value. 
			if (std::chrono::duration_cast<std::chrono::minutes>(std::chrono::steady_clock::now() - begin).count() == 5)
			{
				std::cout << "5 Minutes elapsed.";
				begin = std::chrono::steady_clock::now();
				m_closed = !m_closed;
				std::cout << "Counter is " << !m_closed << " now.";
			}


			// If counter is closed then wait till it is opened.
			if (m_closed)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				continue;
			}


			std::unique_lock<std::mutex> lck(m);
			cond.wait(lck, [&] {return !tickets.empty(); });	
			getFirstTicketinQueue();
			lck.unlock();
			cond.notify_all();
			
			std::cout << "Counter [" << m_currentTicket.getTicketType() << "] serving  " << m_currentTicket.getDuration() << std::endl;
			std::this_thread::sleep_for(std::chrono::seconds(m_currentTicket.getDuration()));
		}
}

void Counter::getFirstTicketinQueue()
{
		// Print all ticket's (duration) in the queue. 
		std::cout << "Counter [" << m_counterType << "] Queue->   ";
		for (auto& x : tickets)
		{
			std::cout << x.getDuration() << " ";
		}
		std::cout << std::endl;
		// Get the first ticket in the queue. 
		m_currentTicket = tickets.front();
		tickets.pop_front();
}

Ticket& Counter::getCurrentTicket() 
{
	return m_currentTicket;
}


int main()
{

	// Here we need to create number of waiting lists for each ticket type. 
	TicketQueue typeATickets;
	TicketQueue typeBTickets;
	TicketQueue typeCTickets;

	// Ticket generator accepts the waiting lists and generates the tickets.
	TicketGenerator tp(typeATickets, typeBTickets, typeCTickets);

	// Counters can be created depending on ticket types. (Multiple counters of same ticket can exist).
	Counter c1(typeATickets, TicketType::A);
	Counter c2(typeBTickets, TicketType::B);
	Counter c3(typeCTickets, TicketType::C);

	//Add more counters if you want to have it for same ticket type (for example A)
	//Counter c11(typeATickets, TicketType::A);
	//Counter c12(typeATickets, TicketType::A);	
	//Counter c13(typeATickets, TicketType::A);


	// Create threads for each counters and generator.
	std::thread tpThread(tp);
	std::thread c1Thread(c1);
	std::thread c2Thread(c2);
	std::thread c3Thread(c3);
	//std::thread c11Thread(c11);
	//std::thread c12Thread(c12);
	//std::thread c13Thread(c13);

	tpThread.join();
	c1Thread.join();
	c2Thread.join();
	c3Thread.join();
	//c11Thread.join();
	//c12Thread.join();
	//c13Thread.join();

}
