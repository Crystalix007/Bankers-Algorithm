#include <cassert>
#include <iomanip>
#include <iostream>
#include <queue>
#include <set>
#include <vector>

class Owner;

using ResourceCount = uint32_t;
using Resources = std::vector<ResourceCount>;
using PrioritisedOwnership = std::multiset<Owner>;

void zero(Resources& resources);
Resources operator+(const Resources& left, const Resources& right);
Resources operator-(const Resources& left, const Resources& right);
bool operator<(const Resources& left, const Resources& right);
bool operator<(const PrioritisedOwnership& left,
               const PrioritisedOwnership& right);

struct Owner {
private:
	bool completed;
	Resources owned;
	Resources required;

public:
	Owner(Resources owned, Resources required)
	    : completed{ true }, owned{ owned }, required{ required } {
		assert(owned.size() == required.size());

		for (const auto& elem : required) {
			if (elem != 0)
				this->completed = false;
		}
	}

	const Resources& getRequired() const {
		return this->required;
	}

	Resources allocate() {
		Resources freed = required + owned;
		zero(required);
		zero(owned);
		completed = true;

		return freed;
	}

	bool operator<(const Owner& other) const {
		assert(required.size() == other.required.size());

		if (completed != other.completed)
			return completed;

		return required < other.required;
	}

	bool isComplete() const {
		return completed;
	}
};

class Configuration {
private:
	PrioritisedOwnership owned;
	Resources free;

public:
	Configuration(const std::vector<Owner>& owned, const Resources& free)
	    : owned{ owned.begin(), owned.end() }, free{ free } {
		assert(owned.size() > 0);
	}

	bool operator<(const Configuration& other) const {
		assert(owned.size() == other.owned.size());

		if (free < other.free)
			return true;
		if (other.free < free)
			return false;

		return owned < other.owned;
	}

	bool operator>(const Configuration& other) const {
		assert(owned.size() == other.owned.size());

		if (other.free < free)
			return true;
		if (free < other.free)
			return false;

		return other.owned < owned;
	}

	bool isComplete() const {
		for (const auto& owner : owned) {
			if (!owner.isComplete())
				return false;
		}

		return true;
	}

	const PrioritisedOwnership& getOwned() const {
		return owned;
	}

	Resources getFree() const {
		return free;
	}
};

bool isValid(const Configuration& startConfig);

int main() {
	std::cout << "Allocating resources: " << std::endl;
	Owner a{ { 0, 0, 0 }, { 5, 7, 9 } };
	Owner b{ { 0, 2, 0 }, { 1, 3, 4 } };
	Configuration initial{ { a, b }, { 5, 2, 3 } };

	std::cout << "Is configuration valid: " << std::boolalpha
	          << isValid(initial) << std::endl;

	return 0;
}

void zero(Resources& resources) {
	for (auto& resource : resources) {
		resource = 0;
	}
}

Resources operator+(const Resources& left, const Resources& right) {
	assert(left.size() == right.size());

	Resources result = left;

	for (size_t i = 0; i < result.size(); i++) {
		result[i] += right[i];
	}

	return result;
}

Resources operator-(const Resources& left, const Resources& right) {
	assert(left.size() == right.size());

	Resources result = left;

	for (size_t i = 0; i < result.size(); i++) {
		result[i] -= right[i];
	}

	return result;
}

bool operator<(const Resources& left, const Resources& right) {
	assert(left.size() == right.size());

	for (size_t i = 0; i < left.size(); i++) {
		using RType = std::make_signed_t<ResourceCount>;

		RType result = left[i] - right[i];

		if (result != 0)
			return result < 0;
	}

	return false;
}

bool operator<(const PrioritisedOwnership& left,
               const PrioritisedOwnership& right) {
	assert(left.size() == right.size());

	auto leftElem = left.begin();
	auto rightElem = right.begin();

	while (leftElem != left.end()) {
		if (*leftElem < *rightElem)
			return true;
		if (*rightElem < *leftElem)
			return false;

		leftElem++;
		rightElem++;
	}

	return false;
}

bool isValid(const Configuration& startConfig) {
	std::priority_queue<Configuration, std::vector<Configuration>,
	                    std::greater<Configuration>>
	    configurations{};
	configurations.push(startConfig);

	while (!configurations.empty() && !configurations.top().isComplete()) {
		bool proceeded = false;
		auto c = configurations.top();
		auto free = c.getFree();
		auto owned = c.getOwned();
		configurations.pop();
		const std::vector<Owner> simpleOwned{ owned.begin(), owned.end() };

		for (size_t i = 0; i < simpleOwned.size(); i++) {
			if (simpleOwned[i].isComplete())
				continue;

			if (free < simpleOwned[i].getRequired())
				continue;

			proceeded = true;

			std::vector<Owner> newOwned{ simpleOwned };
			auto freed = newOwned[i].allocate();

			configurations.push({ newOwned, free + freed });
		}

		if (!proceeded)
			break;
	}

	return !configurations.empty();
}
