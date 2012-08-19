#include "stdafx.h"
#include "taskNetUser.h"
#include "exitBtn.h"
#include "taskSelect.h"

MsgSignUp::MsgSignUp(lw::HTTPClient* pClient, const char* name, const char* password)
:lw::HTTPMsg(L"lwSignUp", pClient){
	lwassert(name && password);
	_buff.writeUtf8(name);
	_buff.writeUtf8(password);
}

MsgSignUp::~MsgSignUp(){

}

void MsgSignUp::onRespond(){
	taskNetUser.onSignUp(_buff);
}

MsgLogin::MsgLogin(lw::HTTPClient* pClient, const char* name, const char* password)
:lw::HTTPMsg(L"lwLogIn", pClient){
	lwassert(name && password);
	_buff.writeUtf8(name);
	_buff.writeUtf8(password);
}

MsgLogin::~MsgLogin(){

}

void MsgLogin::onRespond(){
	taskNetUser.onLogin(_buff);
}

void TaskNetUser::vBegin(){
	_pExitBtn = new ExitBtn(this);
	_text = lw::UIText::create("arial.fnt");
	_text->setPos(0, 445);
	_text->setSize(320, 40);

	int x1 = 10;
	int y = 120;
	int w = 120;
	int h = 20;
	int x2 = x1 + w + 5;
	int x3 = x2 + w + 5;
	_signupName = new lw::TextField(x1, y, w, h);
	_signupPassword = new lw::TextField(x2, y, w, h);
	_btnSignup = lw::Button::createButton9("common", "calibri20.fnt");
	_btnSignup->setPos(x3, y);
	_btnSignup->setSize(55, 20);
	_btnSignup->setText(L"singUp");
	_btnSignup->setTextColor(lw::COLOR_BLACK);
	_btnSignup->setCallback(this);

	y += 30;
	_loginName = new lw::TextField(x1, y, w, h);
	_loginPassword = new lw::TextField(x2, y, w, h);
	_btnLogin = lw::Button::createButton9("common", "calibri20.fnt");
	_btnLogin->setPos(x3, y);
	_btnLogin->setSize(55, 20);
	_btnLogin->setText(L"login");
	_btnLogin->setTextColor(lw::COLOR_BLACK);
	_btnLogin->setCallback(this);

	_pHTTPClient = new lw::HTTPClient(L"localhost", 8082);
}

void TaskNetUser::vEnd(){
	delete _pExitBtn;
	delete _text;
	delete _signupName;
	delete _signupPassword;
	delete _btnSignup;
	delete _loginName;
	delete _loginPassword;
	delete _btnLogin;
	delete _pHTTPClient;
}

void TaskNetUser::vMain(float dt){
	_pHTTPClient->main();
}

void TaskNetUser::vDraw(float dt){
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glDisable(GL_LIGHTING);
	glDepthMask(GL_FALSE);
}

bool TaskNetUser::vOnTouchEvent(std::vector<lw::TouchEvent>& events){
	lw::TouchEvent& evt0 = events[0];
	if ( evt0.type == lw::TouchEvent::TOUCH ){
		if ( evt0.x > 460 && evt0.y < 20 ){
			stop();
			taskSelect.start(0);
			return true;
		}
	}
	return false;
}

void TaskNetUser::onButtonClick(lw::Button* pButton){
	if ( pButton == _btnSignup ){
		std::wstringstream ss;
		lw::HTTPMsg* pMsg = new MsgSignUp(_pHTTPClient, _signupName->getUtf8(), _signupPassword->getUtf8());
		_pHTTPClient->sendMsg(pMsg);
	}else if ( pButton == _btnLogin ){
		std::wstringstream ss;
		lw::HTTPMsg* pMsg = new MsgLogin(_pHTTPClient, _loginName->getUtf8(), _loginPassword->getUtf8());
		_pHTTPClient->sendMsg(pMsg);
	}
}

void TaskNetUser::onSignUp(lw::HttpMsgBuf& buf){
	int n = buf.readInt();
	if ( n == 0 ){
		_text->setText(L"signup failed");
		return;
	}
	UTF82W name(buf.readString());
	UTF82W password(buf.readString());
	std::wstringstream ss;
	ss << buf.readString();
	_text->setText(ss.str().c_str());
}

void TaskNetUser::onLogin(lw::HttpMsgBuf& buf){
	int n = buf.readInt();
	if ( n == 0 ){
		_text->setText(L"account is not exist");
		return;
	}
	if ( n == -1 ){
		_text->setText(L"wrong password");
		return;
	}
	std::wstringstream ss;
	ss << buf.readString();
	_text->setText(ss.str().c_str());
}

TaskNetUser taskNetUser;