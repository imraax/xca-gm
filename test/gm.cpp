/* vi: set sw=4 ts=4:
 *
 * XCA-GM: Chinese GM/T (SM2 / SM3 / SM4) end-to-end tests
 *
 * All rights reserved.
 */

#include <QTest>
#include <QDir>

#include <openssl/x509.h>

#include "lib/pki_evp.h"
#include "lib/pki_x509.h"
#include "lib/pki_x509req.h"
#include "lib/pki_crl.h"
#include "lib/pki_pkcs12.h"
#include "lib/pki_multi.h"
#include "lib/pki_export.h"
#include "lib/db_x509.h"
#include "lib/db_key.h"
#include "lib/database_model.h"
#include "lib/x509name.h"
#include "lib/x509v3ext.h"
#include "lib/digest.h"
#include "lib/xfile.h"
#include "lib/settings.h"

#include "widgets/MainWindow.h"
#include "main.h"

void test_main::gmSM2()
{
#ifndef XCA_GM
	QSKIP("GM/T SM2/SM3/SM4 support not compiled in");
#else
	try {

	ign_openssl_error();
	openDB();
	QDir d; d.mkpath("testdata");

	/* 1. Generate an SM2 key pair */
	keyjob job("SM2");
	QVERIFY(job.isValid());
	QVERIFY(job.isSM2());
	QCOMPARE(job.toString(), "SM2");

	pki_evp *key = new pki_evp("SM2 CA key");
	key->generate(job);
	openssl_error();
	QVERIFY(key->isSM2());
	QVERIFY(key->isECbased());
	QVERIFY(key->isPrivKey());
	QCOMPARE(key->getKeyType(), (int)EVP_PKEY_SM2);
	QCOMPARE(key->getKeyTypeString(), "SM2");
	QCOMPARE(key->length(), "256 bit");
	QCOMPARE(key->ecParamNid(), (int)NID_sm2);
	/* SM2 only signs together with SM3 */
	QCOMPARE(key->possibleHashNids(), QList<int>{ NID_sm3 });
	Database.insert(key);
	dbstatus();

	/* 2. Self signed CA certificate, signature SM2-with-SM3 */
	x509name xn;
	xn.addEntryByNid(NID_countryName, "CN");
	xn.addEntryByNid(NID_commonName, "XCA-GM SM2 Test CA");

	pki_x509 *ca = new pki_x509("SM2 CA");
	ca->setSubject(xn);
	ca->setIssuer(xn);
	ca->setPubKey(key);
	ca->setSerial(a1int(1L));
	ca->setNotBefore(a1time(a1time::now()));
	ca->setNotAfter(a1time(a1time::now(365 * 24 * 3600)));
	x509v3ext bc;
	QVERIFY(ca->addV3ext(bc.create(NID_basic_constraints, "critical,CA:TRUE")));
	QVERIFY(ca->addV3ext(x509v3ext().create(NID_key_usage,
				"critical,keyCertSign,cRLSign")));
	ca->sign(key, digest(NID_sm3));
	openssl_error();
	QCOMPARE(ca->getSigAlg(), "SM2-with-SM3");
	QCOMPARE(X509_get_signature_nid(ca->getCert()), (int)NID_SM2_with_SM3);
	QCOMPARE(X509_verify(ca->getCert(), key->getPubKey()), 1);
	Database.insert(ca);
	dbstatus();
	QVERIFY(ca->isCA());
	QVERIFY(ca->getRefKey() == key);
	QVERIFY(ca->getSigner() == ca);

	/* 3. Certificate signing request signed with SM2-with-SM3 */
	pki_evp *eekey = new pki_evp("SM2 endentity key");
	eekey->generate(job);
	Database.insert(eekey);
	x509name een;
	een.addEntryByNid(NID_countryName, "CN");
	een.addEntryByNid(NID_commonName, "sm2.example.cn");
	pki_x509req *req = new pki_x509req("SM2 request");
	req->createReq(eekey, een, digest(NID_sm3), extList());
	openssl_error();
	QCOMPARE(req->getSigAlg(), "SM2-with-SM3");
	QVERIFY(req->verify());
	Database.insert(req);

	/* 4. End entity certificate issued by the SM2 CA */
	pki_x509 *ee = new pki_x509("SM2 endentity");
	ee->setSubject(een);
	ee->setIssuer(xn);
	ee->setPubKey(eekey);
	ee->setSerial(a1int(2L));
	ee->setNotBefore(a1time(a1time::now()));
	ee->setNotAfter(a1time(a1time::now(30 * 24 * 3600)));
	ee->sign(key, digest(NID_sm3));
	openssl_error();
	QCOMPARE(ee->getSigAlg(), "SM2-with-SM3");
	QCOMPARE(X509_verify(ee->getCert(), key->getPubKey()), 1);
	QCOMPARE(X509_verify(ee->getCert(), eekey->getPubKey()), 0);
	ign_openssl_error();
	Database.insert(ee);
	dbstatus();
	QVERIFY(ee->getSigner() == ca);

	/* 5. CRL signed with SM2-with-SM3 */
	pki_crl *crl = new pki_crl();
	crl->createCrl("SM2 CRL", ca);
	crl->setLastUpdate(a1time(a1time::now()));
	crl->setNextUpdate(a1time(a1time::now(7 * 24 * 3600)));
	crl->sign(key, digest(NID_sm3));
	openssl_error();
	QCOMPARE(crl->getSigAlg(), "SM2-with-SM3");
	Database.insert(crl);

	/* 6. Export the key: PEM, PKCS#8 encrypted with SM4-CBC */
	db_base *keys = Database.model<db_key>();
	QVERIFY(keys != nullptr);
	QModelIndexList list;
	list << keys->index(keys->getByName("SM2 endentity key"));
	const pki_export *xport = pki_export::by_id(20); /* PEM private */
	QVERIFY(xport != nullptr);
	QString file = "testdata/sm2key.pem";
	{
		XFile F(file);
		F.open_write();
		keys->exportItems(list, xport, F);
	}
	xport = pki_export::by_id(53); /* PKCS #8 SM4 encrypted */
	QVERIFY(xport != nullptr);
	QVERIFY(xport->match_all(F_SM4 | F_PKCS8 | F_CRYPT));
	pwdialog->setExpectations(QList<pw_expect*>{
		new pw_expect("sm4pass", pw_ok),
		new pw_expect("sm4pass", pw_ok),
	});
	QString pk8 = "testdata/sm2key.pk8";
	{
		XFile F(pk8);
		F.open_write();
		keys->exportItems(list, xport, F);
	}
	/* Re-import the PEM key and compare */
	{
		pki_multi *pems = new pki_multi();
		pems->probeAnything(file);
		QCOMPARE(pems->get().size(), 1);
		pki_key *k = dynamic_cast<pki_key*>(pems->get()[0]);
		QVERIFY(k != nullptr);
		QVERIFY(k->isSM2());
		QVERIFY(k->isPrivKey());
		QVERIFY(k->compare(eekey));
		delete pems;
	}
	/* Re-import the SM4 encrypted PKCS#8 key */
	{
		QFile f(pk8);
		QVERIFY(f.open(QIODevice::ReadOnly));
		QByteArray content = f.readAll();
		QVERIFY(content.contains("-----BEGIN ENCRYPTED PRIVATE KEY-----"));
		pwdialog->setExpectations(QList<pw_expect*>{
			new pw_expect("sm4pass", pw_ok),
		});
		pki_evp *k = new pki_evp();
		k->fromPEMbyteArray(content, pk8);
		openssl_error();
		QVERIFY(k->isSM2());
		QVERIFY(k->compare(eekey));
		delete k;
	}

	/* 7. PKCS#12 with SM4-CBC and HMAC-SM3 */
	db_base *certs = Database.model<db_x509>();
	QVERIFY(certs != nullptr);
	list.clear();
	list << certs->index(certs->getByName("SM2 endentity"));
	Settings["pkcs12_enc_algo"] = encAlgo(NID_sm4_cbc).name();
	QCOMPARE(encAlgo((QString)Settings["pkcs12_enc_algo"]).isSM4(), true);
	pwdialog->setExpectations(QList<pw_expect*>{
		new pw_expect("p12pass", pw_ok),
		new pw_expect("p12pass", pw_ok),
	});
	QString p12 = "testdata/sm2.p12";
	{
		XFile F(p12);
		F.open_write();
		certs->exportItems(list, pki_export::by_id(14), F); /* chain */
	}
	pwdialog->setExpectations(QList<pw_expect*>{
		new pw_expect("p12pass", pw_ok),
	});
	{
		pki_pkcs12 *p = new pki_pkcs12(p12);
		openssl_error();
		QVERIFY(p->getKey() != nullptr);
		QVERIFY(p->getKey()->isSM2());
		QVERIFY(p->getKey()->compare(eekey));
		QCOMPARE(p->getCert()->getSigAlg(), "SM2-with-SM3");
		QVERIFY(p->getCert()->compare(ee));
		/* HMAC-SM3 integrity protection */
		QMap<QString, QString> prp;
		p->collect_properties(prp);
		QVERIFY2(prp["Algorithm"].toUpper().startsWith("SM3"),
			QString("PKCS#12 MAC algorithm: '%1'")
				.arg(prp["Algorithm"]).toUtf8());
		delete p;
	}

	} catch (errorEx &e) {
		QString msg = QString("Exception: %1").arg(e.getString());
		QVERIFY2(false, msg.toUtf8().constData());
	} catch (enum open_result r) {
		QVERIFY2(false, QString("Password error %1").arg(r).toUtf8());
	}
#endif
}
