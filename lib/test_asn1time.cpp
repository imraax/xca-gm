/* vi: set sw=4 ts=4:
 *
 * Copyright (C) 2023 Christian Hohnstaedt.
 *
 * All rights reserved.
 */

#include <QTest>
#include <QLocale>
#include <QTimeZone>

#include "asn1time.h"

#include <stdlib.h>
       #include <time.h>
class test_asn1time: public QObject
{
    Q_OBJECT
private slots:
    void construct_op();
    void output();
};

void test_asn1time::construct_op()
{
	a1time b, a("20191125153015Z");
	QVERIFY(a.isValid());
	QVERIFY(!a.isUndefined());
	QVERIFY(b != a);
	QVERIFY(b > a);
	b = a;
	QCOMPARE(b , a);
	a1time c(a.get());
	QCOMPARE(c , a);
	a1time d(a.get_utc());
	QCOMPARE(d , a.toUTC());
}

void test_asn1time::output()
{
	QLocale::setDefault(QLocale::C);
#if !defined(Q_OS_WIN32)
	setenv("TZ","UTC", 1);
	tzset();
#endif

	a1time a("20191125153015Z");

	QCOMPARE(a.toString("yyyy MM"), "2019 11");
	QCOMPARE(a.toSortable(), "2019-11-25");

	QCOMPARE(a.toPlain(),  "20191125153015Z");
	QCOMPARE(a.toPlainUTC(), "191125153015Z");

#if !defined(Q_OS_WIN32)
	/* The abbreviation of the UTC zone ("UTC" or "GMT") depends on the
	 * Qt version and on the time zone database of the platform */
	auto pretty_is = [](const a1time &t, const QString &expect) {
		QString pretty = t.toPretty();
		QString zone = pretty.section(' ', -1);
		QVERIFY2(zone == "UTC" || zone == "GMT",
			qPrintable(QString("Unexpected zone in '%1'").arg(pretty)));
		QCOMPARE(pretty.section(' ', 0, -2), expect);
	};
	a.setTimeZone(QTimeZone("UTC"));
	pretty_is(a, "Monday, 25 November 2019 15:30:15");

	a.setTimeZone(QTimeZone("Europe/Berlin"));
	pretty_is(a, "Monday, 25 November 2019 14:30:15");

	a.setTimeZone(QTimeZone("UTC+07:00"));
	pretty_is(a, "Monday, 25 November 2019 08:30:15");
#endif
}

QTEST_MAIN(test_asn1time)
#include "test_asn1time.moc"
