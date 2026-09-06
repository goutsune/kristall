#include "nextextrenderer.hpp"
#include "renderhelpers.hpp"

#include <cstring>
#include <QList>
#include <QTextCursor>

std::unique_ptr<QTextDocument> NexTextRenderer::render(
    const QByteArray &input,
    const QUrl &root_url,
    const DocumentStyle &themed_style)
{
    QTextCharFormat standard;
    standard.setFont(themed_style.preformatted_font);
    standard.setForeground(themed_style.preformatted_color);

    QTextCharFormat standard_link;
    standard_link.setFont(themed_style.preformatted_font);
    standard_link.setForeground(QBrush(themed_style.internal_link_color));

    std::unique_ptr<QTextDocument> result = std::make_unique<QTextDocument>();
    renderhelpers::setPageMargins(result.get(), themed_style.margin_h, themed_style.margin_v);

    QTextCursor cursor{result.get()};

    QTextCharFormat text_fmt = standard;

    QList<QByteArray> lines = input.split('\n');
    for (int i = 0; i < lines.size(); i++)
    {
        QByteArray line = lines.at(i);
        if (line.endsWith('\r'))
            line.chop(1);

        if (i == lines.size() - 1 && line.isEmpty())
            break;

        if (line.startsWith("=>"))
        {
            const char *data = line.constData();
            int start = 2 + std::strspn(data + 2, " \t");
            int end = start + std::strcspn(data + start, " \t");

            QString url = QString::fromUtf8(line.mid(start, end - start));
            QString link_text = QString::fromUtf8(line.mid(start)).trimmed();

            if (not url.isEmpty())
            {
                QTextCharFormat fmt = standard_link;
                fmt.setAnchor(true);
                fmt.setAnchorHref(root_url.resolved(QUrl(url)).toString());

                cursor.insertText(QString::fromUtf8(line.left(start)), standard);
                cursor.insertText(link_text + "\n", fmt);
                continue;
            }
        }

        renderhelpers::renderEscapeCodes(line + "\n", text_fmt, standard, cursor);
    }

    return result;
}
