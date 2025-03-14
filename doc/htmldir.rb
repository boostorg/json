# coding: utf-8

require 'asciidoctor'
require 'asciidoctor/converter/html5'
require 'pathname'

class Asciidoctor::AbstractBlock
    attr_accessor :real_level

    def document_dirname
        if @document.upper_doc && (@document.upper_doc != @document.top_doc)
            base = @document.upper_doc.document_filepath()
            base.delete_suffix @document.attributes['outfilesuffix']
        else
            "."
        end
    end

    def document_filename
        base = @document.id
        if @document.upper_doc && (@document.upper_doc != @document.top_doc)
            base = base.delete_prefix "#{document.upper_doc.id}_"
        end
        base + @document.attributes['outfilesuffix']
    end

    def document_filepath
        suffix = @document.attributes['outfilesuffix']
        File.join(document_dirname, @document.document_filename).delete_prefix(
            ".#{File::SEPARATOR}")
    end

    def filepath_to other
        self_dir = Pathname.new document_dirname
        other_dir = Pathname.new other.document_dirname
        ret = File.join(
            other_dir.relative_path_from(self_dir),
            other.document_filename).delete_prefix(".#{File::SEPARATOR}")
        ret
    end
end

class Asciidoctor::Document
    attr_accessor :parsed
    attr_accessor :processed
    attr_accessor :catalog
    attr_accessor :child_documents
    attr_accessor :top_doc
    attr_accessor :upper_doc
    attr_accessor :next_doc
    attr_accessor :prev_doc

    alias core_init initialize
    def initialize data = nil, options = {}
        options[:sourcemap] = true
        core_init data, options
        @child_documents = []
    end

    def content # this is different from the reguar Document.content
        super
    end

    alias core_docinfo docinfo
    def docinfo location = :head, suffix = nil
        result = core_docinfo location, suffix
        if ((location == :footer) &&
                (attr? 'source-location', nil, true) &&
                (attr? 'project-gh', nil, true))

            result += '<div class="edit-link"><div class="edit-link-content">'
            result += %(<a href="#{attr 'project-gh'}/#{attr 'source-location'})
            result += '">Edit this page</a></div></div>'
        end
        if [:header, :footer].include?(location)
            result += '<div class="nav-links"><div class="nav-links-content">'
            if location == :header
                if self.upper_doc
                    links = []
                    ancestor = self
                    while ancestor
                        links.prepend(
                            nav_link_html(ancestor, 'nav-link nav-crumb'))
                        ancestor = ancestor.upper_doc
                    end
                    result +=
                        %(<nav id="breadcrumbs"><ul>#{links.join}</ul></nav>)
                end

                block_id = 'siblings-header'
            else # location == :footer
                block_id = 'siblings-footer'
            end

            links = []
            if @prev_doc
                links << nav_link_html(@prev_doc, 'nav-link nav-prev')
            end
            if @next_doc
                links << nav_link_html(@next_doc, 'nav-link nav-next')
            end
            result += %(<nav id="#{block_id}"><ul>#{links.join}</ul></nav>)
            result += '</div></div>'
        end
        result
    end

    def link_to_target(refid, target = "##{refid}")
        doc = @catalog[:refs][refid]
        until !doc || (doc.context == :document)
            doc = doc.parent
        end
        if !doc || (doc.id == @id)
            target
        elsif "##{doc.id}" == target
            @document.filepath_to doc
        else
            "#{@document.filepath_to doc}#{target}"
        end
    end

    def nav_link_html(target, role)
        text = target.captioned_title
        title = target.document.doctitle sanitize: true
        text = %(<span class="title">#{text}</span>)
        link = link_to_target target.id
        link = %(<a href="#{link}" title="#{title}">#{text}</a>)
        %(<li class="#{role}">#{link}</li>)
    end

    def spawn_child_documents
        pagelevels = attr('pagelevels', 0).to_i
        if pagelevels < 0
            logger.warn "attribute 'pagelevels' must be >= 0; setting to 0"
            pagelevels = 0
        end
        set_attribute('pagelevels', pagelevels.to_s)

        @id = File.basename(
            @attributes['outfile'] , @attributes['outfilesuffix'])
        register(
            :refs,
            [@id,
             Asciidoctor::Inline.new(
                 parent = self,
                 context = :anchor,
                 text = doctitle,
                 opts = {:type => :ref, :id => @id}),
             doctitle])

        @child_documents = generate_child_docs self, pagelevels
        @real_level = 0

        if sections.empty?
            attrs = @attributes.clone
            @blocks << Asciidoctor::Section.new(
                self,
                1,
                false,
                {:attributes => attrs})
        end
        reindex_sections
        update_source_location

        @top_doc = self
        make_linear_connections
        update_real_level self, 0

        @processed = true
    end

    def generate_child_docs(node, doc_pagelevels)
        result = []
        node.blocks.filter! do |block|
            if block.context != :section
                next true
            end

            node.document.playback_attributes block.attributes
            pagelevels = block.attr('pagelevels', doc_pagelevels).to_i
            if block.level > pagelevels || !block.id
                next true
            end

            result << node.document.spawn_document_from(block, doc_pagelevels)
            false
        end
        result
    end

    def spawn_document_from(node, doc_pagelevels)
        pagelevels = node.attr('pagelevels', doc_pagelevels).to_i

        attrs = @attributes.clone
        attrs['title'] = node.title
        attrs['notitle'] = true
        attrs['authors'] = nil
        attrs['author'] = nil
        attrs['pagelevels'] = doc_pagelevels
        doc = Asciidoctor::Document.new(
            [],
            {:attributes => attrs,
             :doctype => self.doctype,
             :header_footer => !self.attr?(:embedded),
             :safe => self.safe})
        doc.id = node.id
        doc.catalog = @catalog
        doc.child_documents = generate_child_docs(node, pagelevels)
        doc.blocks << node
        doc.finalize_header({})
        reparent_blocks(node, doc)
        doc.real_level = node.real_level
        doc.reindex_sections
        doc.update_source_location
        doc.parsed = true
        doc.processed = true
        doc
    end

    def update_real_level(node, real_level)
        node.real_level = real_level

        if node.context == :dlist
            node.find_by(context: :list_item).each do |block|
                update_real_level block, real_level + 1
            end
        else
            node.blocks.each do |block|
                update_real_level block, real_level + 1
            end
        end
    end

    def reparent_blocks(node, parent, real_level = node.level)
        node.parent = parent
        node.real_level = real_level
        node.level = node.level - real_level + 1

        if node.context == :dlist
            node.find_by(context: :list_item).each do |block|
                reparent_blocks(block, node, real_level + 1)
            end
            return
        end

        node.blocks.each do |block|
            reparent_blocks(block, node, real_level + 1)
            if block.context == :table
                block.columns.each do |col|
                    col.parent = col.parent
                end
                block.rows.body.each do |row|
                    row.each do |cell|
                        cell.parent = cell.parent
                    end
                end
            end
        end
    end

    def make_linear_connections
        prev = self
        child_documents.each do |doc|
            doc.top_doc = @top_doc
            doc.upper_doc = self
            doc.prev_doc = prev
            prev.next_doc = doc
            prev = doc.make_linear_connections
        end
        prev
    end

    def update_source_location
        if (first_section.attr? 'source-location', nil, true)
            set_attribute(
                'source-location', first_section.attr('source-location'))
            return
        end

        if !first_section.source_location
            return
        end

        path = Pathname.new first_section.source_location.file
        if (attr? 'project-dir', nil, true)
            path = path.relative_path_from(attr 'project-dir')
        end

        set_attribute('source-location', path.to_s)
    end
end

class HtmlDirConverter < Asciidoctor::Converter::Html5Converter
    include Asciidoctor
    include Asciidoctor::Converter
    include Asciidoctor::Writer

    register_for 'htmldir'

    attr_accessor :home_doc

    def convert_document(node)
        if !node.processed
            @home_doc = node
            node.spawn_child_documents
        end

        return super
    end

    def convert_outline(node, opts = {})
        convert_outline_doc node.document, node.top_doc, opts
    end

    def convert_outline_doc(doc, node, opts)
        toclevels = ((node.first_section.attr 'toclevels') ||
            opts[:toclevels] || (node.document.attr 'toclevels', 2)).to_i
        return unless node.real_level < toclevels

        opts[:toclevels] = toclevels
        if node.first_section.id == node.id
            sections = node.first_section.sections
        else
            sections = node.sections
        end
        children = sections + node.child_documents
        convert_outline_ext doc, children, node.real_level + 1, opts
    end

    def convert_outline_sect(doc, node, opts)
        toclevels = ((node.attr 'toclevels').to_i || opts[:toclevels] || 2)
        return unless node.real_level < toclevels

        opts[:toclevels] = toclevels
        convert_outline_ext doc, node.sections, node.real_level + 1, opts
    end

    def convert_outline_ext(doc, nodes, level, opts)
        return unless !nodes.empty?

        sectnumlevels = (
            opts[:sectnumlevels] ||
            (nodes[0].document.attributes['sectnumlevels'] || 3).to_i)

        result = [%(<ul class="sectlevel#{level}">)]
        nodes.each do |node|
            result << list_item_for_node(doc, node, sectnumlevels)
            if node.context == :document
                child_toc_level = convert_outline_doc doc, node, opts
            else
                child_toc_level = convert_outline_sect doc, node, opts
            end
            result << child_toc_level if child_toc_level
        end
        result << '</ul>'
        result.join LF
    end

    def list_item_for_node(doc, node, sectnumlevels)
        if node.context == :document
            section = node.sections[0]
        else
            section = node
        end

        slevel = section.real_level
        signifier = nil
        sectnum = nil
        if !section.caption && (
                section.numbered && slevel <= sectnumlevels)
            if slevel < 2 && node.document.doctype == 'book'
                case section.sectname
                when 'chapter'
                    signifier = (
                        node.document.attributes['chapter-signifier'])
                    sectnum = section.sectnum
                when 'part'
                    signifier = (
                        node.document.attributes['part-signifier'])
                    sectnum = section.sectnum nil, ':'
                else
                    sectnum = section.sectnum
                end
            end
        end
        signifier = signifier ? "#{signifier} " : ''
        sectnum = sectnum ? "#{sectnum} " : ''
        stitle = %(#{signifier}#{sectnum}#{section.title})
        stitle = stitle.gsub DropAnchorRx, '' if stitle.include? '<a'
        if section.id == doc.id
            role = ' class="this-page"'
            target = "#"
        else
            role = ""
            target = doc.link_to_target section.id
        end
        %(<li><a href="#{target}"#{role}>#{stitle}</a>)
    end

    def convert_inline_anchor(node)
        if node.type != :xref
            return super
        end

        if (path = node.attributes['path'])
            attrs = append_link_constraint_attrs(
                node, node.role ? [%( class="#{node.role}")] : [])
            attrs = attrs.join
            text = node.text || path
        else
            attrs = node.role ? %( class="#{node.role}") : ''
            unless (text = node.text)
                ref = (@refs ||= node.document.catalog[:refs])[
                    refid = node.attributes['refid']] ||
                    (refid.nil_or_empty? ? (top = get_root_document node) : nil)
                if AbstractNode === ref
                    if (@resolving_xref ||= (outer = true)) && outer
                        if (text = ref.xreftext node.attr 'xrefstyle', nil, true)
                            text = text.gsub DropAnchorRx, '' if text.include? '<a'
                        else
                            text = top ? '[^top]' : %([#{refid}])
                        end
                        @resolving_xref = nil
                    else
                        text = top ? '[^top]' : %([#{refid}])
                    end
                else
                    text = %([#{refid}])
                end
            end
        end

        target = node.document.link_to_target(
            node.attributes['refid'], node.target)
        %(<a href="#{target}"#{attrs}>#{text}</a>)
    end

    def write output, target
        File.write target, output, mode: Asciidoctor::FILE_WRITE_MODE
        write_children(
            @home_doc, File.dirname(target), File.extname(target))
    end

    def write_children(doc, outdir, ext)
        doc.child_documents.each do |child|
            outfile = File.join(outdir, child.document_filepath)

            outfiledir = File.dirname(outfile)
            FileUtils.mkdir_p(outfiledir) unless File.exist?(outfiledir)

            File.open(outfile, 'w') do |f|
                logger.info %(Writing to '#{outfile}')
                f.write(child.convert)
            end
            write_children(child, outdir, ext)
        end
    end
end
