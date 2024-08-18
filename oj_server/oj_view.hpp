#pragma once

#include <iostream>
#include <vector>
#include <ctemplate/template.h>
#include <string>

#include "oj_model.hpp"
#include <ctemplate/template_dictionary.h>

namespace ns_view
{
    using namespace ns_model;
    const std::string template_path = "./question_html/";
    class View
    {
    public:
        View() {}
        ~View() {}

        // html：输出型参数
        bool AllQuestionHtml(const std::vector<struct Question> &questions, std::string *html)
        {
            if (questions.empty())
            {
                return false;
            }
            ctemplate::TemplateDictionary root("questions");

            for (const auto &q : questions)
            {
                ctemplate::TemplateDictionary *sub = root.AddSectionDictionary("questions_list");
                sub->SetValue("number", q.number);
                sub->SetValue("title", q.title);
                sub->SetValue("level", q.level);
            }
            // 渲染
            ctemplate::Template *tpl = ctemplate::Template::GetTemplate(template_path + "Allquestions.html", ctemplate::DO_NOT_STRIP);
            // 渲染->
            tpl->Expand(html, &root);
            return true;
        }
        void OneQuestionHtml(const struct Question &question, std::string *html)
        {
            ctemplate::TemplateDictionary root("question");
            root.SetValue("number", question.number);
            root.SetValue("title", question.title);
            root.SetValue("desc", question.desc);
            root.SetValue("model_code", question.header);
            root.SetValue("level", question.level);

            ctemplate::Template *ctl = ctemplate::Template::GetTemplate(template_path + "Onequestion.html", ctemplate::DO_NOT_STRIP);

            ctl->Expand(html, &root);
        }
    };
}