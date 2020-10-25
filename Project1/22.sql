select type, count(type)
from Pokemon
group by type
order by count(type), type
