#include <QDir>
#include <QDirIterator>
#include <iostream>

using namespace std;

int main(int argc, const char** argv )
{
    cout << "<?xml version=\"1.0\" encoding=\"utf-8\"?>";
    cout << "<sphinx:docset>";
    cout << "<sphinx:schema>";
    cout << "<sphinx:field name=\"c\" attr=\"string\"/>";
    cout << "<sphinx:field name=\"f\" attr=\"string\"/>";
    cout << "<sphinx:attr name=\"i\" attr=\"int\"  bits=\"32\"/>";
    cout << "</sphinx:schema>";

    QDir path( argv[1]  );
    QDirIterator it( path.path(),  QStringList() << "*.txt", QDir::AllDirs|QDir::Files, QDirIterator::Subdirectories );
    int i = 0;
    while (it.hasNext())
    {
        it.next();
        if ( it.fileInfo().isDir() )
        {
            continue;
        }

	cout << "<sphinx:document id=\"" << i+1 << "\">";
	cout << "<c>";
	//cat %%i
        QFile file(it.filePath());
        file.open(QIODevice::ReadOnly);
        QByteArray content = file.readAll();
        size_t contentSize = content.size();
        content[(int)contentSize-1]= '\0';
        cout << content.data();
	cout << "</c>";
	cout << "<f>" << it.fileName().toStdString() << "</f>";
	cout << "<i>" << QFileInfo(it.fileName()).baseName().toInt() << "</i>";
	cout << "</sphinx:document>";
    
        i++;
    }

    cout << "</sphinx:docset>";
}
