#include "stdafx.h"
#include "bullseyeRound.h"
#include "targetPod.h"
#include "targetBullseye.h"
#include "soundManager.h"

const int ROUND_NUM = 5;

BullseyeRound* BullseyeRound::create(int round, std::list<Target*>* pTargets){
	BullseyeRound* p = NULL;
	switch (round)
	{
	case 1:
		p = new BullseyeRound1(pTargets);
		break;
	case 2:
		p = new BullseyeRound2(pTargets);
		break;
	case 3:
		p = new BullseyeRoundFluit(pTargets);
		break;
	case 4:
		p = new BullseyeRound3(pTargets);
		break;
	case 5:
		p = new BullseyeRound4(pTargets);
		break;
	
	}
	return p;
}

BullseyeRound::~BullseyeRound(){
}

BullseyeRound1::BullseyeRound1(std::list<Target*>* pTargets){
	TargetBullseye* pBullseye = new TargetBullseye(1.0f);
	pBullseye->setPos(0.f, 400.f);
	pBullseye->setAngle(0.f /*(float)M_PI/6.f*/);
	pTargets->push_back(pBullseye);
	_pBullseye = pBullseye;
}

void BullseyeRound1::vMain(float dt){
	
}


BullseyeRound2::BullseyeRound2(std::list<Target*>* pTargets){
	TargetBullseye* pBullseye = new TargetBullseye(0.5f);
	pBullseye->setPos(0.f, 400.f);
	pBullseye->setAngle(0.f /*(float)M_PI/6.f*/);
	pTargets->push_back(pBullseye);
	_pBullseye = pBullseye;
}

void BullseyeRound2::vMain(float dt){
	
}

BullseyeRound3::BullseyeRound3(std::list<Target*>* pTargets)
:_t(0.f){
	TargetBullseye* pBullseye = new TargetBullseye(1.f);
	pBullseye->setPos(0.f, 400.f);
	pBullseye->setAngle(0.f /*(float)M_PI/6.f*/);
	pTargets->push_back(pBullseye);
	_pBullseye = pBullseye;
}

void BullseyeRound3::vMain(float dt){
	_t += 0.04f;
	_pBullseye->setPos(sin(_t)*120.f, 400.f);
}

BullseyeRound4::BullseyeRound4(std::list<Target*>* pTargets)
:_t(0.f){
	_pBullseye = new TargetBullseye(0.5f);
	_pBullseye->setPos(0.f, 400.f);
	_pBullseye->setAngle(0.f /*(float)M_PI/6.f*/);
	pTargets->push_back(_pBullseye);
}

void BullseyeRound4::vMain(float dt){
	_t += 0.04f;
	_pBullseye->setPos(sin(_t)*120.f, 400.f);
}

BullseyeRoundFluit::BullseyeRoundFluit(std::list<Target*>* pTargets)
:_t(0.f){
	_pBullseye = new TargetBullseye(1.f);
	_pBullseye->setPos(0.f, 400.f);
	_pBullseye->setAngle(0.f /*(float)M_PI/6.f*/);
	pTargets->push_back(_pBullseye);

	_pFluit = new TargetPod("fluit.pod", "apple", g_pSndHit);
	_pFluit->setPos(0.f, 250.f);
	_pFluit->setScore(0.f);
	pTargets->push_back(_pFluit);
}

void BullseyeRoundFluit::vMain(float dt){
	
}