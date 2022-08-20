TEMPLATES=$(find osv/templates -type f -name '*.html' )

index_head () 
{
	cat "osv/templates/base/top"
}

index_bottom () 
{
	cat "osv/templates/base/bottom"
}

template_tag ()
{
	echo "<script type='template/handlebars' data-template-path='/$1'>"
	cat $1;
	echo '</script>'
}

index_body ()
{
	for template in $TEMPLATES; do
		template_tag $template
	done
}

build_index ()
{
	index_head;
	index_body;
	index_bottom;
}

build_index

