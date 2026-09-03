#ifndef NEXTEXTRENDERER_HPP
#define NEXTEXTRENDERER_HPP

#include "documentstyle.hpp"

#include <memory>
#include <QTextDocument>

struct NexTextRenderer
{
    NexTextRenderer() = delete;


    //! Renders the given byte sequence into a QTextDocument.
    //! @param input    The utf8 encoded input string
    //! @param root_url The url that is used to resolve relative links
    //! @param style    The style which is used to render the document
    static std::unique_ptr<QTextDocument> render(
        QByteArray const & input,
        QUrl const & root_url,
        DocumentStyle const & style
    );
};

#endif // NEXTEXTRENDERER_HPP
