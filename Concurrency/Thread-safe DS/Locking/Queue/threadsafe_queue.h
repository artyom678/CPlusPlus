#pragma once
#include <mutex>
#include <condition_variable>
#include <memory>

namespace threadsafe {


	template<typename T>
	class queue {

		struct Node;

		std::unique_ptr<Node> head;
		Node* tail;
		std::mutex head_mutex;
		std::mutex tail_mutex;
		std::condition_variable empty;

		Node* get_tail();
		std::unique_ptr<Node> pop_head();
		std::unique_ptr<Node> try_pop_head(T& value);
		std::unique_ptr<Node> try_pop_head();
		std::unique_lock<std::mutex> wait_for_data();
		std::unique_ptr<Node> wait_pop_head();
		std::unique_ptr<Node> wait_pop_head(T& value);
	public:
		explicit queue();
		 
		queue(const queue&) = delete;

		queue& operator=(const queue&) = delete;

		std::shared_ptr<T> try_pop();

		bool try_pop(T& value);

		std::shared_ptr<T> wait_and_pop();

		void wait_and_pop(T& value);
		
		void push(T value);
		
	};



	template<typename T>
	struct queue<T>::Node {
		std::shared_ptr<T> value;
		std::unique_ptr<Node> next;
		explicit Node() noexcept
		{}
		explicit Node(T val) noexcept
			: value(std::make_shared<T>(std::move(value)))
		{}
	};

	template<typename T>
	typename queue<T>::Node* queue<T>::get_tail() {
		std::lock_guard lg(tail_mutex);
		return tail;
	}

	template<typename T>
	queue<T>::queue() 
		: head(std::make_unique<Node>())
		, tail(head.get())
	{}

	template<typename T>
	void queue<T>::push(T value) {

		auto new_value = std::make_shared<T>(std::move(value));
		auto new_tail = std::make_unique<Node>();
		Node* p = new_tail.get();
		{
			std::lock_guard lg(tail_mutex);

			tail->value = std::move(new_value);
			tail->next = std::move(new_tail);
			tail = p;
		}
		empty.notify_one();
	}


	template<typename T>
	std::unique_ptr<typename queue<T>::Node> queue<T>::pop_head() {
		auto old_head = std::move(head);
		head = std::move(old_head->next);
		return old_head;
	}


	template<typename T>
	std::unique_ptr<typename queue<T>::Node> queue<T>::try_pop_head() {
		std::lock_guard lg(head_mutex);
		std::unique_ptr<Node> old_head;
		if (head.get() != get_tail()) {
			old_head = pop_head();
		}
		return old_head;
	}

	template<typename T>
	std::unique_ptr<typename queue<T>::Node> queue<T>::try_pop_head(T& value) {
		std::lock_guard lg(head_mutex);
		std::unique_ptr<Node> old_head;
		if (head.get() != get_tail()) {
			value = std::move(*head->value);
			old_head = pop_head();
		}
		return old_head;
	}

	template<typename T>
	std::shared_ptr<T> queue<T>::try_pop() {
		auto old_head = try_pop_head();
		return old_head->value;
	}

	template<typename T>
	bool queue<T>::try_pop(T& value) {
		auto old_head = try_pop_head(value);
		return (bool)old_head; // for some reason compiler doesn't want to perform an implicit cast
	}

	template<typename T>
	std::unique_lock<std::mutex> queue<T>::wait_for_data() {
		std::unique_lock lk(head_mutex);
		empty.wait(lk, [this] {return head.get() != get_tail(); });
		return lk;
	}

	template<typename T>
	std::unique_ptr<typename queue<T>::Node> queue<T>::wait_pop_head() {
		auto lk = wait_for_data();
		return pop_head();
	}

	template<typename T>
	std::unique_ptr<typename queue<T>::Node> queue<T>::wait_pop_head(T& value) {
		auto lk = wait_for_data();
		value = std::move(*head->value);
		return pop_head();
	}

	template<typename T>
	std::shared_ptr<T> queue<T>::wait_and_pop() {
		auto old_head = wait_pop_head();
		return old_head->value;
	}

	template<typename T>
	void queue<T>::wait_and_pop(T& value) {
		auto old_head = wait_pop_head(value); 
	}

};
