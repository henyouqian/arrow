#ifndef __TASK_NETUSER_H__
#define __TASK_NETUSER_H__

class ExitBtn;

class MsgSignUp : public lw::HTTPMsg{
public:
	MsgSignUp(lw::HTTPClient* pClient, const char* name, const char* password);
	~MsgSignUp();

	virtual void onRespond();
};

class MsgLogin : public lw::HTTPMsg{
public:
	MsgLogin(lw::HTTPClient* pClient, const char* name, const char* password);
	~MsgLogin();

	virtual void onRespond();
};

class TaskNetUser : public lw::Task, public lw::ButtonCallback{
public:
	virtual void vBegin();
	virtual void vEnd();
	virtual void vMain(float dt);
	virtual void vDraw(float dt);
	virtual bool vOnTouchEvent(std::vector<lw::TouchEvent>& events);
	virtual void onButtonClick(lw::Button* pButton);

	void onSignUp(lw::HttpMsgBuf& buf);
	void onLogin(lw::HttpMsgBuf& buf);

private:
	ExitBtn* _pExitBtn;
	lw::UIText* _text;
	lw::TextField* _signupName;
	lw::TextField* _signupPassword;
	lw::Button* _btnSignup;
	lw::TextField* _loginName;
	lw::TextField* _loginPassword;
	lw::Button* _btnLogin;
	lw::HTTPClient* _pHTTPClient;
};

extern TaskNetUser taskNetUser;

#endif //__TASK_NETUSER_H__