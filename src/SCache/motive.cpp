#include "motive.h"

class Combinator{
	int* state;
	size_t N, K;
public:
	Combinator(size_t _N, size_t _K):
		state(0), N(_N), K(_K)
	{}
	~Combinator() { if(state != 0) { delete[] state; } }
	const int* GetState() const { return state; }
	bool Next(){
		if(N < K){
			return false;
		}
		if(state == 0){
			state = new int[K];
			for(int i = 0; i < K; i++){
				state[i] = i;
			}
			return true;
		}
		for(int i = K - 1; i >= 0; i--){
			if(state[i] < N - K + i){
				state[i]++;
				for(int j = i + 1; j < K; j++){
					state[j] = state[j - 1] + 1;
				}
				return true;
			}
		}
		return false;
	}
};

TMotiveData::TMotiveData():
	trafficSize(0)//,
	//objectsSize(0)
{}

TMotiveData::TMotiveData(const std::string& firstId, size_t size):
	trafficSize(size)//,
	//objectsSize(size)
{
	ids[firstId] = std::make_pair(1, size);;
}

MotiveTable::MotiveTable():
	divisor(0),
	depth(0)
{}

void MotiveTable::makeFixedSets(const TObjectAttr& attrib,
		size_t size) const
{
	Combinator comb(attrib.size(), size);
	while(comb.Next()){
		const int* cur = comb.GetState();
		TObjectAttr m;
		// fill current set using Combinator indices
		for(int i = 0; i < size; i++){
			m.push_back(attrib[cur[i]]);
		}
		currentSets.push_back(m);
	}
}

void MotiveTable::makeAllSets(const TObjectAttr& attrib) const
{
	currentSets.clear();
	for(int i = 1; i <= depth; i++){
		makeFixedSets(attrib, i);
	}
}

void MotiveTable::SetDepth(int _depth)
{
	depth = _depth;
}

void MotiveTable::AddRequest(const std::string& id,
		const TObjectAttr& attrib, size_t size)
{
	makeAllSets(attrib);
	for(auto& motiveSet: currentSets){
		if(table.find(motiveSet) != table.end()){
			auto& cell = table[motiveSet];
			cell.trafficSize += size;
			if(cell.ids.find(id) == cell.ids.end()){
				//cell.objectsSize += size;
				cell.ids[id] = std::make_pair(1, size);
			}else{
				cell.ids[id].first++;
			}
		}else{
			table[motiveSet] = TMotiveData(id, size);
		}
	}
	divisor += size;
}


void MotiveTable::Recalculate(std::vector<TMotive>& result,
		size_t maxSegments, double minPrecision)
{
	// sort motives
	std::vector<TMotive> sortedMotives;
	for(auto& m: table){
		sortedMotives.push_back(std::make_pair(m.first, m.second));
	}
	std::sort(sortedMotives.begin(), sortedMotives.end(),
		[](auto& a, auto& b){
			return a.second.trafficSize > b.second.trafficSize;
			//return (double)a.second.trafficSize / a.second.objectsSize >
			//		(double)b.second.trafficSize / b.second.objectsSize;
		});
	// return by threshold
	size_t counter = maxSegments;
	for(auto& m: sortedMotives){
		if(((double)m.second.trafficSize) / divisor < minPrecision
				|| counter-- == 0)
		{
			break;
		}
		result.push_back(m);
	}
}

void MotiveTable::Compress(double quotient)
{
	if(quotient == 1.0){
		return;
	}
	for(auto& elem: table){
		elem.second.trafficSize *= quotient;
	}
	divisor *= quotient;
}

size_t MotiveTable::GetDivisor() const
{
	return divisor;
}

TPushArray MotiveTable::GetPrePush(const TObjectAttr& attrib)
{
	TPushArray result;
	for(auto& it: table[attrib].ids){
		result.push_back(std::make_pair(it.first, it.second));
	}
	std::sort(result.begin(), result.end(),
		[](auto& a, auto& b){ return a.second.first > b.second.first; });

	return result;
}
