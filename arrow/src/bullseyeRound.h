#ifndef __BULLSEYE_ROUND_H__
#define __BULLSEYE_ROUND_H__

class TargetBullseye;
class TargetPod;
class Target;

extern const int ROUND_NUM;

class BullseyeRound{
public:
	static BullseyeRound* create(int round, std::list<Target*>* pTargets);
	virtual ~BullseyeRound();

	virtual void vMain(float dt) = 0;

protected:
	
};

class BullseyeRound1 : public BullseyeRound{
public:
	BullseyeRound1(std::list<Target*>* pTargets);
	virtual void vMain(float dt);

private:
	TargetBullseye* _pBullseye;
};

class BullseyeRound2 : public BullseyeRound{
public:
	BullseyeRound2(std::list<Target*>* pTargets);
	virtual void vMain(float dt);

private:
	TargetBullseye* _pBullseye;
};

class BullseyeRound3 : public BullseyeRound{
public:
	BullseyeRound3(std::list<Target*>* pTargets);
	virtual void vMain(float dt);

private:
	float _t;
	TargetBullseye* _pBullseye;
};

class BullseyeRound4 : public BullseyeRound{
public:
	BullseyeRound4(std::list<Target*>* pTargets);
	virtual void vMain(float dt);

private:
	float _t;
	TargetBullseye* _pBullseye;
};

class BullseyeRoundFluit : public BullseyeRound{
public:
	BullseyeRoundFluit(std::list<Target*>* pTargets);
	virtual void vMain(float dt);

private:
	float _t;
	TargetBullseye* _pBullseye;
	TargetPod* _pFluit;
};

#endif //__BULLSEYE_ROUND_H__