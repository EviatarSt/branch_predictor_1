/* 046267 Computer Architecture - Spring 2020 - HW #1 */
/* This file should hold your implementation of the predictor simulator */

#include "bp_api.h"

typedef struct {
	entrance* BTB;
	FSM_case* FSMs_local;
	FSM_case** FSM_global;
	int BTB_Size;
	int TagSize;
	int HistorySize;
	int Globals;//MSB indicates global history, LSB global S.M. table
	int Share;
}Branch_Predictor;

typedef struct {
	unsigned IP;
	unsigned branch;
	unsigned history : 8;
	unsigned valid : 1;
} entrance;

typedef struct {
	unsigned FSM : 2;
}FSM_case;

Branch_Predictor* the_BP;//global BP, for the run.

int BP_init(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
			bool isGlobalHist, bool isGlobalTable, int Shared){
	the_BP = (Branch_Predictor*)malloc(sizeof(Branch_Predictor));
	the_BP->BTB_Size = btbSize;
	the_BP->TagSize = tagSize;
	the_BP->HistorySize = historySize;
	the_BP->Share = Shared;
	int counter = 2 ^ historySize;
	//allocating the tables, according to the globality of the history and the fsm.
	if (isGlobalHist && isGlobalTable) {
		the_BP->Globals = 3;
		the_BP->FSM_global = (FSM_case*)malloc((counter) * sizeof(FSM_case));
		the_BP->BTB = (entrance*)malloc(btbSize*sizeof(entrance);
	}
	else if (isGlobalHist && !isGlobalTable) {
		the_BP->Globals = 2;
		the_BP->BTB = (entrance*)malloc(btbSize * sizeof(entrance);
		the_BP->FSMs_local = (FSM_case**)malloc(btbSize*sizeof(FSM_case*));
	}
	else if (!isGlobalHist && isGlobalTable) {
		the_BP->Globals = 1;
		the_BP->FSM_global = (FSM_case*)malloc((counter) * sizeof(FSM_case));
		the_BP->BTB = (entrance*)malloc(btbSize * sizeof(entrance);
	}
	else {
		the_BP->Globals = 0;
		the_BP->FSMs_local = (FSM_case**)malloc(btbSize * sizeof(FSM_case*));
		the_BP->BTB = (entrance*)malloc(btbSize * sizeof(entrance);
	}
	
	FSM_case* temp;
	if (the_BP->Globals == (0 || 2)) {
		for (int i = 0; i < btbSize; i++) {
			the_BP->FSMs_local[i] = (FSM_case*)malloc((counter) * size(FSM_case));
			for (int j = 0; j < counter; j++) {
				temp = the_BP->FSMs_local[i];
				temp[j].FSM = fsmState;
			}
		}
	}
	if (the_BP->Globals == (1 || 3)) {
		for (int j = 0; j < counter; j++) {
			*temp = the_BP->FSM_global[j];
			temp->FSM = fsmState;
		}
	}
	return 1;
}

unsigned createMask(unsigned a, unsigned b)//we need to add a declaration somewhere
{
	unsigned r = 0;
	for (unsigned i = a; i < b; i++)
		r |= 1 << i;

	return r;
}

bool BP_predict(uint32_t pc, uint32_t *dst){
	int ind_length = log(the_BP->BTB_Size) / log(2);
	unsigned mask1 = createMask(2, 2 + ind_length);
	unsigned index = (ind_length == 0) ? 0 : (mask1 & pc);
	int tag_size = the_BP->TagSize;
	unsigned mask2 = createMask(2 + ind_length, tag_size + ind_length + 2);//is the tag coming as needed? make sure of it.
	unsigned tag = pc & mask2;
	entrance check = the_BP->BTB[index];
	unsigned second_tag = check.IP & mask2;
	if ((second_tag != tag) || (check.valid == 0)) {
		return false;
	}
	unsigned mask3 = createMask(0, the_BP->HistorySize + 1), mask4;
	//side section, to get the "shared" value to get xor from.
	unsigned to_xor;
	mask4 = 0;
	if (the_BP->Share == 1) {
		unsigned end = the_BP->HistorySize + 2 + 1;
		mask4 = createMask(2, end);
		to_xor = (pc & mask4) >> 2
	}
	if (the_BP->Share == 2) {
		unsigned end = the_BP->HistorySize + 16 + 1;
		mask4 = createMask(16, end);
		to_xor = (pc & mask4) >> 16
	}
	if (the_BP->Globals == 0) {//local history and local FSMs
		unsigned fsm_index = (check.history & mask3);
		FSM_case* temp = the_BP->FSMs_local[index];
		unsigned check = temp[fsm_index].FSM;
		if (check < 2) { return false; }
		return true;
	}
	if (the_BP->Globals == 1) {//local history, global FSMs.
		unsigned fsm_index = (the_BP->Share != 0) ? (check.history & !to_xor) : check.history;
		FSM_case temp = the_BP->FSM_global[fsm_index];
		unsigned fsm = temp.FSM;
		if (check < 2) { return false; }
		return true;
	}
	if (the_BP->Globals == 2) {//global history, local FSMs
		entrance first = the_BP->BTB[0];
		unsigned fsm_index = first.history;
		FSM_case* temp = the_BP->FSMs_local[index];
		unsigned check = temp[fsm_index].FSM;
		if (check < 2) { return false; }
		return true;
	}
	if (the_BP->Globals == 3) {//global history, FSMs
		entrance first = the_BP->BTB[0];
		unsigned fsm_index = (the_BP->Share != 0) ? (first.history & !to_xor) : first.history;
		FSM_case temp = the_BP->FSM_global[fsm_index];
		unsigned fsm = temp.FSM;
		if (check < 2) { return false; }
		return true;
	}
	return false;
}

void BP_update(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst){
	return;
}

void BP_GetStats(SIM_stats *curStats){
	return;
}

