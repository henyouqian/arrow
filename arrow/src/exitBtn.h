#ifndef __EXIT_BUTTON_H__
#define __EXIT_BUTTON_H__

class ExitBtn: public lw::ButtonCallback{
public:
	ExitBtn(lw::Task* pTask);
	~ExitBtn();
	virtual void onButtonClick(lw::Button* pButton);

private:
	lw::Task* _pTask;
	lw::Button* _pBtn;
};


#endif //__EXIT_BUTTON_H__