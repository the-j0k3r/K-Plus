//============================================================================
//
//   File : optw_identity.cpp
//   Creation date : Tue Nov 14 2000 23:06:53 CEST by Szymon Stefanek
//
//   This file is part of the KVirc irc client distribution
//   Copyright (C) 2000 Szymon Stefanek (pragma at kvirc dot net)
//
//   This program is FREE software. You can redistribute it and/or
//   modify it under the terms of the GNU General Public License
//   as published by the Free Software Foundation; either version 2
//   of the License, or (at your opinion) any later version.
//
//   This program is distributed in the HOPE that it will be USEFUL,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//   See the GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program. If not, write to the Free Software Foundation,
//   Inc. ,59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
//============================================================================

#include "optw_identity.h"

#include "kvi_defaults.h"
#include "kvi_settings.h"
#include "kvi_options.h"
#include "kvi_locale.h"
#include "kvi_app.h"
#include "kvi_window.h"
#include "kvi_console.h"
#include "kvi_optionswidget.h"
#include "kvi_filedialog.h"
#include "kvi_fileextensions.h"
#include "kvi_iconmanager.h"
#include "kvi_http.h"
#include "kvi_tal_tooltip.h"

#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QLayout>
#include <QTabWidget>
#include <QComboBox>
#include <QMessageBox>
#include <QTimer>
#include <QValidator>
#include <QCloseEvent>

KviNickAlternativesDialog::KviNickAlternativesDialog(QWidget * par,const QString &n1,const QString &n2,const QString &n3)
: QDialog(par)
{
	QGridLayout * g = new QGridLayout(this);

	setWindowTitle(__tr2qs_ctx("Nickname alternatives","options"));

	QLabel * l = new QLabel(this);
	l->setText(__tr2qs_ctx("<center>Here you can choose up to three nicknames " \
		"alternative to the primary one. KVIrc will use the alternatives " \
		"if the primary nick is already used by someone else on a particular " \
		"IRC network.</center>","options"));
	g->addMultiCellWidget(l,0,0,0,2);

	l = new QLabel(this);
	l->setText(__tr2qs_ctx("Alt. Nickname 1:","options"));
	g->addWidget(l,1,0);
	m_pNickEdit1 = new QLineEdit(this);
	g->addMultiCellWidget(m_pNickEdit1,1,1,1,2);
	m_pNickEdit1->setText(n1);

	l = new QLabel(this);
	l->setText(__tr2qs_ctx("Alt. Nickname 2:","options"));
	g->addWidget(l,2,0);
	m_pNickEdit2 = new QLineEdit(this);
	g->addMultiCellWidget(m_pNickEdit2,2,2,1,2);
	m_pNickEdit2->setText(n2);

	l = new QLabel(this);
	l->setText(__tr2qs_ctx("Alt. Nickname 3:","options"));
	g->addWidget(l,3,0);
	m_pNickEdit3 = new QLineEdit(this);
	g->addMultiCellWidget(m_pNickEdit3,3,3,1,2);
	m_pNickEdit3->setText(n3);
	KviTalHBox * h = new KviTalHBox(this);
	h->setSpacing(8);
	g->addWidget(h,4,2);

	QPushButton * pb = new QPushButton(__tr2qs_ctx("Cancel","options"),h);
	//g->addWidget(pb,4,2);
	connect(pb,SIGNAL(clicked()),this,SLOT(reject()));

	pb = new QPushButton(__tr2qs_ctx("Ok","options"),h);
	pb->setDefault(true);
	connect(pb,SIGNAL(clicked()),this,SLOT(accept()));

	g->setColumnStretch(0,1);

	//setMinimumSize(250,120);
}

KviNickAlternativesDialog::~KviNickAlternativesDialog()
{
}

void KviNickAlternativesDialog::fill(QString &n1,QString &n2,QString &n3)
{
	n1 = m_pNickEdit1->text();
	n2 = m_pNickEdit2->text();
	n3 = m_pNickEdit3->text();
}


KviAvatarDownloadDialog::KviAvatarDownloadDialog(QWidget * par,const QString &szUrl)
: QDialog(par)
{
	setWindowTitle(__tr2qs_ctx("Avatar Download - KVIrc","options"));

	m_szUrl = szUrl;

	QGridLayout * g = new QGridLayout(this);

	m_pOutput = new QLabel(__tr2qs_ctx("<center>Please wait while the avatar is being downloaded</center>","options"),this);
	g->addMultiCellWidget(m_pOutput,0,0,0,1);

	QPushButton * b = new QPushButton(__tr2qs_ctx("Abort","options"),this);
	g->addWidget(b,1,1);
	connect(b,SIGNAL(clicked()),this,SLOT(cancelClicked()));

	m_pRequest = new KviHttpRequest();

	QTimer::singleShot(0,this,SLOT(startDownload()));

	g->setRowStretch(0,1);
	g->setColumnStretch(0,1);

	setMinimumSize(250,120);
}

KviAvatarDownloadDialog::~KviAvatarDownloadDialog()
{
	delete m_pRequest;
}

void KviAvatarDownloadDialog::startDownload()
{
	connect(m_pRequest,SIGNAL(terminated(bool)),this,SLOT(downloadTerminated(bool)));
	connect(m_pRequest,SIGNAL(status(const char *)),this,SLOT(downloadMessage(const char *)));

	QString tmp = m_szUrl;
	g_pIconManager->urlToCachedFileName(tmp);
	g_pApp->getLocalKvircDirectory(m_szLocalFileName,KviApp::Avatars,tmp);

	m_pRequest->setExistingFileAction(KviHttpRequest::RenameExisting);
	if(!m_pRequest->get(KviUrl(m_szUrl),KviHttpRequest::StoreToFile,m_szLocalFileName.toUtf8().data()))
	{
		m_szErrorMessage = __tr2qs_ctx("Failed to start the download","options");
		reject();
	}
}

void KviAvatarDownloadDialog::closeEvent(QCloseEvent * e)
{
	m_szErrorMessage = __tr2qs_ctx("Download aborted by user","options");
	e->ignore();
	reject();
}

void KviAvatarDownloadDialog::cancelClicked()
{
	m_szErrorMessage = __tr2qs_ctx("Download aborted by user","options");
	reject();
}

void KviAvatarDownloadDialog::downloadMessage(const char * message)
{
	if(message)
	{
		QString txt = "<center>";
		txt += message;
		txt += "</center>";
		m_pOutput->setText(message);
	}
}

void KviAvatarDownloadDialog::downloadTerminated(bool bSuccess)
{
	if(bSuccess)
	{
		accept();
	} else {
		m_szErrorMessage = m_pRequest->lastError();
		reject();
	}
}

KviAvatarSelectionDialog::KviAvatarSelectionDialog(QWidget * par,const QString &szInitialPath)
: QDialog(par)
{
	setWindowTitle(__tr2qs_ctx("Choose Avatar - KVIrc","options"));

	QGridLayout * g = new QGridLayout(this);


	QString msg = "<center>";
	msg += __tr2qs_ctx("Please select an avatar image. " \
				"The full path to a local file or an image on the Web can be used.<br>" \
				"If you wish to use a local image file, click the \"<b>Browse</b>\"" \
				"button to browse local folders.<br>" \
				"The full URL for an image (including <b>http://</b>) can be entered manually.","options");
	msg += "</center><br>";

	QLabel * l = new QLabel(msg,this);
	l->setMinimumWidth(250);

	g->addMultiCellWidget(l,0,0,0,2);

	m_pLineEdit = new QLineEdit(this);
	m_pLineEdit->setText(szInitialPath);
	m_pLineEdit->setMinimumWidth(180);

	g->addMultiCellWidget(m_pLineEdit,1,1,0,1);

	QPushButton * b = new QPushButton(__tr2qs_ctx("&Browse...","options"),this);
	connect(b,SIGNAL(clicked()),this,SLOT(chooseFileClicked()));
	g->addWidget(b,1,2);
	KviTalHBox * h = new KviTalHBox(this);h->setSpacing(8);g->addMultiCellWidget(h,2,2,1,2);
	b = new QPushButton(__tr2qs_ctx("&OK","options"),h);
	b->setMinimumWidth(80);
	b->setDefault(true);
	connect(b,SIGNAL(clicked()),this,SLOT(okClicked()));
	//g->addWidget(b,2,1);

	b = new QPushButton(__tr2qs_ctx("Cancel","options"),h);
	b->setMinimumWidth(80);
	connect(b,SIGNAL(clicked()),this,SLOT(cancelClicked()));
	//g->addWidget(b,2,2);

	g->setRowStretch(0,1);
	g->setColumnStretch(0,1);
	//setMinimumSize(250,120);
}

KviAvatarSelectionDialog::~KviAvatarSelectionDialog()
{
}

void KviAvatarSelectionDialog::okClicked()
{
	m_szAvatarName = m_pLineEdit->text();
	accept();
}

void KviAvatarSelectionDialog::cancelClicked()
{
	reject();
}

void KviAvatarSelectionDialog::chooseFileClicked()
{
	QString tmp;
 	if(KviFileDialog::askForOpenFileName(tmp,__tr2qs_ctx("Choose an Image File - KVIrc","options"),
		QString::null,KVI_FILTER_IMAGE,false,true,this))
	{
		m_pLineEdit->setText(tmp);
	}
}

void KviAvatarSelectionDialog::closeEvent(QCloseEvent * e)
{
	e->ignore();
	reject();
}


KviIdentityOptionsWidget::KviIdentityOptionsWidget(QWidget * parent)
: KviOptionsWidget(parent)
{
}

KviIdentityOptionsWidget::~KviIdentityOptionsWidget()
{
}


KviIdentityGeneralOptionsWidget::KviIdentityGeneralOptionsWidget(QWidget * parent)
: KviOptionsWidget(parent)
{
	m_szAltNicknames[0] = KVI_OPTION_STRING(KviOption_stringNickname2);
	m_szAltNicknames[1] = KVI_OPTION_STRING(KviOption_stringNickname3);
	m_szAltNicknames[2] = KVI_OPTION_STRING(KviOption_stringNickname4);


	createLayout();
	layout()->setMargin(10);

	KviTalGroupBox * gbox = addGroupBox(0,0,0,0,Qt::Horizontal,__tr2qs_ctx("Basic Properties","options"));
	KviTalHBox * hb = new KviTalHBox(gbox);
	hb->setSpacing(3);

	KviStringSelector * sel = addStringSelector(hb,__tr2qs_ctx("Nickname:","options"),KviOption_stringNickname1);
	sel->setMinimumLabelWidth(120);
	mergeTip(sel,__tr2qs_ctx("<center>Your <b>nickname</b> is your primary form of identification on IRC.<br>" \
			"Since servers cannot accept multiple users sharing the same nickname " \
			"(case insensitive), you can provide alternative nicknames to be used in case"\
			"the server refuses to accept the default one.</center>","options"));

	QValidator * v = new QRegExpValidator(QRegExp("[^-0-9 ][^ ]*"),hb);
	sel->setValidator(v);

	QPushButton * pb = new QPushButton(__tr2qs_ctx("Alternatives...","options"),hb);
	connect(pb,SIGNAL(clicked()),this,SLOT(setNickAlternatives()));

	sel = addStringSelector(gbox,__tr2qs_ctx("Username:","options"),KviOption_stringUsername);
	sel->setMinimumLabelWidth(120);
	mergeTip(sel,__tr2qs_ctx("<center>This is the <b>username</b> that you will use to connect to the server.<br>" \
				"In the past, it was used as a form of authentication, but it normally has no special use now.<br>" \
				"In addition to your nickname, you are identified on IRC by your <b>username@hostname</b>.</br>" \
				"Basically, you can enter any word you like here. :D</center>","options"));

	sel = addStringSelector(gbox,__tr2qs_ctx("Real name:","options"),KviOption_stringRealname);
	sel->setMinimumLabelWidth(120);

	mergeTip(sel,__tr2qs_ctx("<center>This text will appear when someone does a /WHOIS on you.<br>" \
				"It is intended to be your real name, but people tend to put random quotes and phrases here too.</center>","options"));


	QString szOptionalCtcpUserInfo = __tr2qs_ctx("This field is optional and will appear as part of the CTCP USERINFO reply.","options");
	QString szCenterBegin("<center>");
	QString szCenterEnd("</center>");
	QString szTrailing = "<br><br>" + szOptionalCtcpUserInfo + szCenterEnd;

	gbox = addGroupBox(0,1,0,1,Qt::Horizontal,__tr2qs_ctx("Profile","options"));

	hb = new KviTalHBox(gbox);
	hb->setSpacing(4);

	QLabel * l = new QLabel(__tr2qs_ctx("Age:","options"),hb);
	l->setMinimumWidth(120);

	m_pAgeCombo = new QComboBox(hb);
	QString szTip1 = szCenterBegin + __tr2qs_ctx("Here you can specify your age.","options") + szTrailing;
	KviTalToolTip::add(l,szTip1);
	KviTalToolTip::add(m_pAgeCombo,szTip1);
	m_pAgeCombo->insertItem(__tr2qs_ctx("Unspecified","options"));
	unsigned int i;
	for(i=1;i<120;i++)
	{
		QString tmp;
		tmp.setNum(i);
		m_pAgeCombo->insertItem(tmp);
	}

	bool bOk;
	i = KVI_OPTION_STRING(KviOption_stringCtcpUserInfoAge).toUInt(&bOk);
	if(!bOk)i = 0;
	if(i > 120)i = 120;
	m_pAgeCombo->setCurrentItem(i);

	hb->setStretchFactor(m_pAgeCombo,1);


	hb = new KviTalHBox(gbox);
	hb->setSpacing(4);

	l = new QLabel(__tr2qs_ctx("Gender:","options"),hb);
	l->setMinimumWidth(120);

	m_pGenderCombo = new QComboBox(hb);
	QString szTip2 =  szCenterBegin + __tr2qs_ctx("Here you can specify your gender.","options") + szTrailing;
	KviTalToolTip::add(l,szTip2);
	KviTalToolTip::add(m_pGenderCombo,szTip2);
	m_pGenderCombo->insertItem(__tr2qs_ctx("Unspecified","options"));
	m_pGenderCombo->insertItem(__tr2qs_ctx("Female","options"));
	m_pGenderCombo->insertItem(__tr2qs_ctx("Male","options"));

	if(KviQString::equalCI(KVI_OPTION_STRING(KviOption_stringCtcpUserInfoGender),"Male"))
		m_pGenderCombo->setCurrentItem(2);
	else if(KviQString::equalCI(KVI_OPTION_STRING(KviOption_stringCtcpUserInfoGender),"Female"))
		m_pGenderCombo->setCurrentItem(1);
	else
		m_pGenderCombo->setCurrentItem(0);

	hb->setStretchFactor(m_pGenderCombo,1);

	sel = addStringSelector(gbox,__tr2qs_ctx("Location:","options"),KviOption_stringCtcpUserInfoLocation);
	sel->setMinimumLabelWidth(120);
	mergeTip(sel,szCenterBegin + __tr2qs_ctx("You can describe here your approximate physical location. " \
				"Something like \"Region, Country\" will be ok. Please note that this information will be viewable " \
				"by anyone so putting more data (like the exact address), generally, <b>is not a good idea</b>.","options") + szTrailing);

	sel = addStringSelector(gbox,__tr2qs_ctx("Languages:","options"),KviOption_stringCtcpUserInfoLanguages);
	sel->setMinimumLabelWidth(120);
	mergeTip(sel,szCenterBegin + __tr2qs_ctx("You can put here the short names of the languages you can speak. " \
				"An example might be \"EN,IT\" that would mean that you speak both Italian and English.","options") + szTrailing);

	sel = addStringSelector(gbox,__tr2qs_ctx("Other:","options"),KviOption_stringCtcpUserInfoOther);
	sel->setMinimumLabelWidth(120);
	mergeTip(sel,szCenterBegin + __tr2qs_ctx("You can put here some additional personal data. " \
				"It might be a funny quote or your homepage url... " \
				"Please note that this information will be viewable " \
				"by anyone so <b>don't put any sensible data</b> (passwords, telephone or credit card numbers).","options") + szTrailing);

	addRowSpacer(0,2,0,2);
}

KviIdentityGeneralOptionsWidget::~KviIdentityGeneralOptionsWidget()
{
}

void KviIdentityGeneralOptionsWidget::setNickAlternatives()
{
	KviNickAlternativesDialog * dlg = new KviNickAlternativesDialog(this,m_szAltNicknames[0],m_szAltNicknames[1],m_szAltNicknames[2]);
	if(dlg->exec() != QDialog::Accepted)return;
	dlg->fill(m_szAltNicknames[0],m_szAltNicknames[1],m_szAltNicknames[2]);
	delete dlg;
}

void KviIdentityGeneralOptionsWidget::commit()
{
	KviOptionsWidget::commit();

	if(KVI_OPTION_STRING(KviOption_stringRealname).isEmpty()) KVI_OPTION_STRING(KviOption_stringUsername)=KVI_DEFAULT_REALNAME;
	if(KVI_OPTION_STRING(KviOption_stringUsername).isEmpty()) KVI_OPTION_STRING(KviOption_stringUsername)=KVI_DEFAULT_USERNAME;

	KVI_OPTION_STRING(KviOption_stringNickname2) = m_szAltNicknames[0];
	KVI_OPTION_STRING(KviOption_stringNickname3) = m_szAltNicknames[1];
	KVI_OPTION_STRING(KviOption_stringNickname4) = m_szAltNicknames[2];


	int i = m_pAgeCombo->currentItem();
	if(i < 0)i = 0;
	if(i > 120)i = 120;
	if(i <= 0)KVI_OPTION_STRING(KviOption_stringCtcpUserInfoAge) = "";
	else KVI_OPTION_STRING(KviOption_stringCtcpUserInfoAge).setNum(i);

	switch(m_pGenderCombo->currentItem())
	{
		case 1:
			// this should be in english
			KVI_OPTION_STRING(KviOption_stringCtcpUserInfoGender) = "Female";
		break;
		case 2:
			// this should be in english
			KVI_OPTION_STRING(KviOption_stringCtcpUserInfoGender) = "Male";
		break;
		default:
			KVI_OPTION_STRING(KviOption_stringCtcpUserInfoGender) = "";
		break;
	}
}


KviIdentityAvatarOptionsWidget::KviIdentityAvatarOptionsWidget(QWidget * parent)
: KviOptionsWidget(parent)
{
	createLayout();
	layout()->setMargin(10);

	m_pLocalAvatar = new KviPixmap(KVI_OPTION_PIXMAP(KviOption_pixmapMyAvatar));

	bool bHaveAvatar = (!KVI_OPTION_STRING(KviOption_stringMyAvatar).isEmpty()) && m_pLocalAvatar->pixmap();

	QString szTip = __tr2qs_ctx("Here you can choose your avatar image. It will be visible<br>" \
		"by other people that request it. Choose a nice image of yourself,<br>" \
		"possibly avoiding obscenity and offending images. It is a good idea<br>" \
		"to choose a relatively small file (say 150 Kb max) because<br>" \
		"most clients have a limit on the size of avatars being downloaded.<br>" \
		"The image also should be smaller than 800x600 pixels since<br>" \
		"it will have to be viewable in everyone's monitor.","options");

	m_pUseAvatarCheck = new KviStyledCheckBox(__tr2qs_ctx("Use avatar","options"),this);
	addWidgetToLayout(m_pUseAvatarCheck,0,0,0,0);
	m_pUseAvatarCheck->setChecked(bHaveAvatar);
	mergeTip(m_pUseAvatarCheck,szTip);

	m_pAvatarPreview = new KviPixmapPreview(this);
	addWidgetToLayout(m_pAvatarPreview,0,1,0,1);
	m_pAvatarPreview->setPixmap(m_pLocalAvatar);
	m_pAvatarPreview->setEnabled(bHaveAvatar);
	connect(m_pUseAvatarCheck,SIGNAL(toggled(bool)),m_pAvatarPreview,SLOT(setEnabled(bool)));
	mergeTip(m_pAvatarPreview,szTip);

	KviTalHBox * hb = new KviTalHBox(this);
	hb->setSpacing(4);
	addWidgetToLayout(hb,0,2,0,2);

	m_pAvatarNameEdit = new QLineEdit(hb);
	m_pAvatarNameEdit->setReadOnly(true);
	m_pAvatarNameEdit->setEnabled(bHaveAvatar);
	m_pAvatarNameEdit->setText(KVI_OPTION_STRING(KviOption_stringMyAvatar));
	connect(m_pUseAvatarCheck,SIGNAL(toggled(bool)),m_pAvatarNameEdit,SLOT(setEnabled(bool)));

	m_pChooseAvatarButton = new QPushButton(__tr2qs_ctx("Choose...","options"),hb);
	m_pChooseAvatarButton->setEnabled(bHaveAvatar);
	connect(m_pUseAvatarCheck,SIGNAL(toggled(bool)),m_pChooseAvatarButton,SLOT(setEnabled(bool)));

	connect(m_pChooseAvatarButton,SIGNAL(clicked()),this,SLOT(chooseAvatar()));

	layout()->setRowStretch(1,2);
}

KviIdentityAvatarOptionsWidget::~KviIdentityAvatarOptionsWidget()
{
	delete m_pLocalAvatar;
}

void KviIdentityAvatarOptionsWidget::commit(void)
{
	KviOptionsWidget::commit();

	QString szAvatarName = m_pAvatarNameEdit->text();

	if(m_pUseAvatarCheck->isChecked() && m_pLocalAvatar->pixmap() && (!szAvatarName.isEmpty()))
	{
		KVI_OPTION_STRING(KviOption_stringMyAvatar) = szAvatarName;
		KVI_OPTION_PIXMAP(KviOption_pixmapMyAvatar) = *m_pLocalAvatar;
		g_pApp->setAvatarFromOptions();
	} else {
		KVI_OPTION_STRING(KviOption_stringMyAvatar) = "";
		KVI_OPTION_PIXMAP(KviOption_pixmapMyAvatar) = KviPixmap();
	}
}

void KviIdentityAvatarOptionsWidget::chooseAvatar()
{
	QString szCurrent = m_pAvatarNameEdit->text();

	KviAvatarSelectionDialog dlg(this,szCurrent);
	if(dlg.exec() != QDialog::Accepted)return;

	szCurrent = dlg.avatarName();
	szCurrent.stripWhiteSpace();

	if(KviQString::equalCIN(szCurrent,"http://",7))
	{
		// this is an url
		// first check if we have it in the cache
		KviAvatar * a = g_pIconManager->getAvatar(QString::null,szCurrent);
		if(!a)
		{
			// no cache... try downloading
			KviAvatarDownloadDialog dlg(this,szCurrent);
			if(dlg.exec() == QDialog::Accepted)
			{
				// downloaded!
				// try to load it
				if(!m_pLocalAvatar->load(dlg.localFileName()))
				{
					// unloadable
					szCurrent = "";
					QMessageBox::warning(this,__tr2qs_ctx("Failed to Load Avatar - KVIrc","options"),
						__tr2qs_ctx("Failed to load the avatar image.<br>" \
							"It may be an unaccessible file or an " \
							"unsupported image format.","options"),QMessageBox::Ok,QMessageBox::NoButton);
				} // else loaded!
			} else {
				// unloadable
				QString tmp;
				KviQString::sprintf(tmp,__tr2qs_ctx("Failed to download the avatar image.<br>" \
					"<b>%Q</b>","options"),&(dlg.errorMessage()));
				QMessageBox::warning(this,__tr2qs_ctx("Avatar Download Failed - KVIrc","options"),tmp,QMessageBox::Ok,QMessageBox::NoButton);
				szCurrent = "";
				m_pLocalAvatar->setNull();
			}
		} else {
			// else OK.. got it in the cache
			m_pLocalAvatar->set(*(a->pixmap()),a->localPath());
			delete a;
		}
	} else {
		// this is a local path
		if(m_pLocalAvatar->load(szCurrent))
		{
			// local path, loaded
			QString tmp = szCurrent;
			int idx = tmp.findRev("/");
			if(idx != -1)
			{
				szCurrent = tmp.right(tmp.length() - (idx + 1));
				tmp = szCurrent;
			}
			idx = tmp.findRev("\\");
			if(idx != -1)
			{
				szCurrent = tmp.right(tmp.length() - (idx + 1));
				tmp = szCurrent;
			}
		} else {
			// unloadable
			QMessageBox::warning(this,__tr2qs_ctx("Failed to Load Avatar - KVIrc","options"),
					__tr2qs_ctx("Failed to load the avatar image.<br>" \
						"It may be an unaccessible file or an " \
						"unsupported image format.","options"),QMessageBox::Ok,QMessageBox::NoButton);
			szCurrent = "";
		}
	}

	if(m_pLocalAvatar->pixmap())
	{
		if((m_pLocalAvatar->pixmap()->width() > 1024) || (m_pLocalAvatar->pixmap()->height() > 768))
		{
			QMessageBox::warning(this,__tr2qs_ctx("Avatar Might Be Too Big - KVIrc","options"),
						__tr2qs_ctx("The avatar you have choosen is bigger than 1024x768 pixels.<br>" \
							"Such a big image will not be seen on all the user monitors<br>" \
							"and will probably be scaled by the remote clients with poor quality<br>" \
							"algorithms to improve performance. You *should* scale it manually<br>" \
							"to a sane size (like 800x600) or choose a different image.","options"));
		}
	}

	m_pAvatarNameEdit->setText(szCurrent);
	m_pAvatarPreview->setPixmap(m_pLocalAvatar);
}


KviIdentityAdvancedOptionsWidget::KviIdentityAdvancedOptionsWidget(QWidget * parent)
: KviOptionsWidget(parent)
{
	m_pISelector=0;
	m_pWSelector=0;
	m_pSSelector=0;

	m_sModeStr = KVI_OPTION_STRING(KviOption_stringDefaultUserMode);
	m_bI = m_sModeStr.contains('i');
	m_bW = m_sModeStr.contains('w');
	m_bS = m_sModeStr.contains('s');

	createLayout();
	layout()->setMargin(10);

	KviTalGroupBox * gbox = addGroupBox(0,0,0,0,Qt::Horizontal,__tr2qs_ctx("User Mode","options"));
	m_pISelector = addBoolSelector(gbox,__tr2qs_ctx("Invisible (+i)","options"),&m_bI);
	m_pSSelector = addBoolSelector(gbox,__tr2qs_ctx("Server notices (+s)","options"),&m_bS);
	m_pWSelector = addBoolSelector(gbox,__tr2qs_ctx("Wallops (+w)","options"),&m_bW);

	gbox = addGroupBox(0,1,0,1,Qt::Horizontal,__tr2qs_ctx("Default Messages","options"));

	KviStringSelector * sel = addStringSelector(gbox, __tr2qs_ctx("Part message:","options"),KviOption_stringPartMessage);
	sel->setMinimumLabelWidth(120);

	mergeTip(sel,__tr2qs_ctx("<center>This is the default part message that will be used when you<br>" \
			"leave a channel by closing a channel window.</center>","options"));

	sel = addStringSelector(gbox, __tr2qs_ctx("Quit message:","options"),KviOption_stringQuitMessage);
	sel->setMinimumLabelWidth(120);
	mergeTip(sel,__tr2qs_ctx("<center>This is the default quit message that will be used when you<br>" \
			"quit your IRC session by closing the console window or disconnecting by pressing the disconnect button.</center>","options"));

	addRowSpacer(0,2,0,2);
}

KviIdentityAdvancedOptionsWidget::~KviIdentityAdvancedOptionsWidget()
{
}

void KviIdentityAdvancedOptionsWidget::commit()
{
	KviOptionsWidget::commit();

	m_sModeStr = m_bI ? "i" : "";
	if(m_bS)m_sModeStr += "s";
	if(m_bW)m_sModeStr += "w";

	KVI_OPTION_STRING(KviOption_stringDefaultUserMode) = m_sModeStr.ptr();
}

#ifndef COMPILE_USE_STANDALONE_MOC_SOURCES
#include "m_optw_identity.moc"
#endif //!COMPILE_USE_STANDALONE_MOC_SOURCES
