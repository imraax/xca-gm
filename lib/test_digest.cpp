/* vi: set sw=4 ts=4:
 *
 * Copyright (C) 2023 Christian Hohnstaedt.
 *
 * All rights reserved.
 */

#include <QTest>
#include <QList>

#include "digest.h"
#include <openssl/evp.h>

class test_digest: public QObject
{
    Q_OBJECT
private slots:
    void default_digest();
    void convert();
    void sm3();
};

void test_digest::default_digest()
{
	digest d(digest::getDefault());
	QCOMPARE(d.name(), "SHA256");
	digest::setDefault("md5");
	QVERIFY(digest::getDefault().isInsecure());
}

void test_digest::convert()
{
	digest d(EVP_sha512());
	digest e("sha512");

	QCOMPARE(d.name(), "SHA512");
	QCOMPARE(d.MD(), e.MD());
	QVERIFY(!d.isInsecure());
	d.adjust(QList<int>({ NID_md5, NID_sha256, NID_sha384 }));
	QCOMPARE(d.name(), "SHA384");

}

void test_digest::sm3()
{
#if !defined(OPENSSL_NO_SM3) && defined(NID_sm3)
	digest d(EVP_sm3());
	digest e("sm3");

	QCOMPARE(d.name(), "SM3");
	QCOMPARE(d.MD(), e.MD());
	QVERIFY(!d.isInsecure());
	QVERIFY(digest::all_digests.contains(NID_sm3));
	/* SM3 is the only digest for SM2 keys */
	d.adjust(QList<int>({ NID_sm3 }));
	QCOMPARE(d.name(), "SM3");
	digest f(EVP_sha256());
	f.adjust(QList<int>({ NID_sm3 }));
	QCOMPARE(f.name(), "SM3");
#else
	QSKIP("SM3 not available in the crypto library");
#endif
}

QTEST_MAIN(test_digest)
#include "test_digest.moc"
